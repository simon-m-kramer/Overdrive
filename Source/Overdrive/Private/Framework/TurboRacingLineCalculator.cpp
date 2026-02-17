// Copyright Simon Kramer. All Rights Reserved.


#include "Framework/TurboRacingLineCalculator.h"
#include "Components/SplineComponent.h"
#include "DrawDebugHelpers.h"

ATurboRacingLineCalculator::ATurboRacingLineCalculator()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	CenterlineSpline = CreateDefaultSubobject<USplineComponent>(TEXT("CenterlineSpline"));
	CenterlineSpline->SetupAttachment(Root);
	CenterlineSpline->SetClosedLoop(true);
	CenterlineSpline->SetDrawDebug(true);
	CenterlineSpline->SetUnselectedSplineSegmentColor(FLinearColor::White);

	RacingLineSpline = CreateDefaultSubobject<USplineComponent>(TEXT("RacingLineSpline"));
	RacingLineSpline->SetupAttachment(Root);
	RacingLineSpline->SetClosedLoop(true);
	RacingLineSpline->SetDrawDebug(true);
	RacingLineSpline->SetUnselectedSplineSegmentColor(FLinearColor::Green);
}

void ATurboRacingLineCalculator::CalculateRacingLine()
{
	CachedPositions.Empty();
	CachedCurvatures.Empty();

	if (!CenterlineSpline)
	{
		UE_LOG(LogTemp, Error, TEXT("RacingLine: No centerline spline."));
		return;
	}

	const float SplineLengthCm = CenterlineSpline->GetSplineLength();
	const int32 NumNodes = FMath::FloorToInt(SplineLengthCm / NodeSpacing);

	if (NumNodes < 4)
	{
		UE_LOG(LogTemp, Error, TEXT("RacingLine: Need at least 4 nodes. Spline length=%.0f cm, spacing=%.0f cm -> %d nodes."),
			SplineLengthCm, NodeSpacing, NumNodes);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("RacingLine: %d nodes over %.0f cm (spacing %.0f cm). Running %d iterations..."),
		NumNodes, SplineLengthCm, NodeSpacing, Iterations);

	// --- Step 1: Sample centerline and set up nodes -------------------------
	//
	// All simulation is done in meters internally to avoid unit-scaling
	// headaches with the spring constants. We convert back to cm at the end.

	const float HalfWidthM = HalfTrackWidth * CmToM;

	TArray<FSimNode> Nodes;
	Nodes.SetNum(NumNodes);

	for (int32 i = 0; i < NumNodes; ++i)
	{
		const float DistanceCm = (static_cast<float>(i) / NumNodes) * SplineLengthCm;

		FVector PosCm = CenterlineSpline->GetLocationAtDistanceAlongSpline(
			DistanceCm, ESplineCoordinateSpace::World);
		FVector TangentDir = CenterlineSpline->GetDirectionAtDistanceAlongSpline(
			DistanceCm, ESplineCoordinateSpace::World);

		// For a 2D (flat) track, the perpendicular is the tangent rotated 90 degrees
		// around the Z axis: (-Y, X, 0).
		FVector Perp(-TangentDir.Y, TangentDir.X, 0.0f);
		Perp.Normalize();

		Nodes[i].CenterPos = PosCm * CmToM;  // Convert to meters
		Nodes[i].Perpendicular = Perp;
		Nodes[i].Offset = 0.0f;
		Nodes[i].Velocity = 0.0f;
	}

	// --- Step 2: Run the spring-hinge simulation ----------------------------
	//
	// For each triplet of consecutive nodes (Prev, Center, Next), the hinge
	// at Center produces forces that try to straighten the bend:
	//
	//   theta*nHat ~= rHat_CN x rHat_CP     (small-angle approximation)
	//   F_next = (k / |r_CN|) * (rHat_CN x theta*nHat)
	//   F_prev = F_next               (by symmetry)
	//   F_center = -2 * F_next        (Newton's third law)
	//
	// CRITICAL SIGN NOTE:
	// The document's eq. 8 can be misread. The correct derivation gives a
	// negative sign: F_next = -(k/|r|)(theta*nHat x rHat). Using the identity
	// -(A x B) = (B x A), we compute CrossProduct(rHat_CN, theta*nHat) which has
	// the correct sign without an explicit negation.

	TArray<FVector> Forces;
	Forces.SetNum(NumNodes);

	for (int32 Iter = 0; Iter < Iterations; ++Iter)
	{
		// Clear accumulated forces
		for (int32 i = 0; i < NumNodes; ++i)
		{
			Forces[i] = FVector::ZeroVector;
		}

		// Accumulate forces from each hinge (triplet)
		const int32 TripletCount = bClosedLoop ? NumNodes : (NumNodes - 2);

		for (int32 t = 0; t < TripletCount; ++t)
		{
			int32 Prev, Center, Next;

			if (bClosedLoop)
			{
				Center = t;
				Prev = (t - 1 + NumNodes) % NumNodes;
				Next = (t + 1) % NumNodes;
			}
			else
			{
				Prev = t;
				Center = t + 1;
				Next = t + 2;
			}

			const FVector PP = Nodes[Prev].GetPosition();
			const FVector PC = Nodes[Center].GetPosition();
			const FVector PN = Nodes[Next].GetPosition();

			const FVector R_CP = PP - PC;  // Center -> Prev
			const FVector R_CN = PN - PC;  // Center -> Next

			const float LenCN = R_CN.Size();
			if (LenCN < KINDA_SMALL_NUMBER)
			{
				continue;
			}

			const FVector R_CP_Hat = R_CP.GetSafeNormal();
			const FVector R_CN_Hat = R_CN.GetSafeNormal();

			// theta * nHat ~= rHat_CN x rHat_CP  (eq. 7 from the document)
			const FVector ThetaN = FVector::CrossProduct(R_CN_Hat, R_CP_Hat);

			// F_next = (k / |r_CN|) * (rHat_CN x theta*nHat)
			// This sign is correct: the double cross product -(theta*nHat x rHat) = (rHat x theta*nHat)
			// pushes the middle node toward the straight line between neighbors.
			const FVector F_Next = (Stiffness / LenCN) * FVector::CrossProduct(R_CN_Hat, ThetaN);

			// Accumulate onto the three nodes of this triplet
			Forces[Prev] += F_Next;          // F_prev = F_next
			Forces[Center] -= 2.0f * F_Next;   // F_center = -2 * F_next
			Forces[Next] += F_Next;           // F_next
		}

		// Apply forces: project onto perpendicular, integrate with Euler method
		for (int32 i = 0; i < NumNodes; ++i)
		{
			// Only allow movement perpendicular to the track (across the road)
			const float ForcePerp = FVector::DotProduct(Forces[i], Nodes[i].Perpendicular);

			// Damped Euler integration (eq. 11 from the document):
			//   v_new = v_old + (F/m - damping * v_old) * dt
			//   x_new = x_old + v_new * dt
			Nodes[i].Velocity += (ForcePerp / Mass - Damping * Nodes[i].Velocity) * DeltaTime;
			Nodes[i].Offset += Nodes[i].Velocity * DeltaTime;

			// Clamp to track boundaries
			if (Nodes[i].Offset > HalfWidthM)
			{
				Nodes[i].Offset = HalfWidthM;
				Nodes[i].Velocity = FMath::Min(Nodes[i].Velocity, 0.0f);
			}
			else if (Nodes[i].Offset < -HalfWidthM)
			{
				Nodes[i].Offset = -HalfWidthM;
				Nodes[i].Velocity = FMath::Max(Nodes[i].Velocity, 0.0f);
			}
		}

		// Log convergence every 10% of iterations
		if (Iter > 0 && Iter % FMath::Max(1, Iterations / 10) == 0)
		{
			float MaxForce = 0.0f;
			float MaxVelocity = 0.0f;
			for (int32 i = 0; i < NumNodes; ++i)
			{
				MaxForce = FMath::Max(MaxForce, Forces[i].Size());
				MaxVelocity = FMath::Max(MaxVelocity, FMath::Abs(Nodes[i].Velocity));
			}
			UE_LOG(LogTemp, Log, TEXT("  Iter %d/%d: MaxForce=%.6f  MaxVelocity=%.6f"),
				Iter, Iterations, MaxForce, MaxVelocity);
		}
	}

	// --- Step 3: Compute curvatures -----------------------------------------
	//
	// The curvature at each node tells us the tightest the path bends there.
	// This is useful later for calculating max cornering speed.
	//   curvature = |rHat_CN x rHat_CP| / |r_CN|    (eq. 9 from the document)

	CachedCurvatures.SetNum(NumNodes);

	for (int32 i = 0; i < NumNodes; ++i)
	{
		int32 Prev = bClosedLoop ? (i - 1 + NumNodes) % NumNodes : FMath::Max(0, i - 1);
		int32 Next = bClosedLoop ? (i + 1) % NumNodes : FMath::Min(NumNodes - 1, i + 1);

		if (Prev == i || Next == i)
		{
			CachedCurvatures[i] = 0.0f;
			continue;
		}

		const FVector PP = Nodes[Prev].GetPosition();
		const FVector PC = Nodes[i].GetPosition();
		const FVector PN = Nodes[Next].GetPosition();

		const FVector R_CP = (PP - PC).GetSafeNormal();
		const FVector R_CN = PN - PC;
		const float LenCN = R_CN.Size();

		if (LenCN < KINDA_SMALL_NUMBER)
		{
			CachedCurvatures[i] = 0.0f;
			continue;
		}

		const FVector R_CN_Hat = R_CN / LenCN;
		const FVector Cross = FVector::CrossProduct(R_CN_Hat, R_CP);

		// Curvature in 1/meters, convert to 1/cm for output
		const float CurvatureM = Cross.Size() / LenCN;
		CachedCurvatures[i] = CurvatureM * CmToM;  // 1/m * (m/cm) = 1/cm
	}

	// --- Step 4: Convert results back to cm and build output spline ---------

	CachedPositions.SetNum(NumNodes);

	RacingLineSpline->ClearSplinePoints(false);

	for (int32 i = 0; i < NumNodes; ++i)
	{
		const FVector WorldPosCm = Nodes[i].GetPosition() * MToCm;
		CachedPositions[i] = WorldPosCm;

		const FVector LocalPos = RacingLineSpline->GetComponentTransform().InverseTransformPosition(WorldPosCm);
		RacingLineSpline->AddSplinePoint(LocalPos, ESplineCoordinateSpace::Local, false);
	}

	RacingLineSpline->SetClosedLoop(bClosedLoop, false);
	RacingLineSpline->UpdateSpline();

	UE_LOG(LogTemp, Log, TEXT("RacingLine: Calculation complete. %d output points."), NumNodes);

	// --- Step 5: Debug visualization ----------------------------------------

	if (bDrawDebugLines && GetWorld())
	{
		FlushPersistentDebugLines(GetWorld());

		for (int32 i = 0; i < NumNodes; ++i)
		{
			const FVector Pos = CachedPositions[i];

			// Racing line segments (green)
			if (bClosedLoop || i < NumNodes - 1)
			{
				const int32 NextIdx = (i + 1) % NumNodes;
				DrawDebugLine(GetWorld(), Pos, CachedPositions[NextIdx],
					FColor::Green, true, DebugLineDuration, 0, 4.0f);
			}

			// Node dots (red = high curvature, blue = low curvature)
			const float CurvNorm = FMath::Clamp(CachedCurvatures[i] * MToCm * 500.0f, 0.0f, 1.0f);
			const FColor NodeColor = FColor::MakeRedToGreenColorFromScalar(1.0f - CurvNorm);
			DrawDebugPoint(GetWorld(), Pos, 10.0f, NodeColor, true, DebugLineDuration);

			// Track boundary indicators
			if (bDrawTrackBounds)
			{
				const FVector CenterCm = Nodes[i].CenterPos * MToCm;
				const FVector LeftEdge = CenterCm + HalfTrackWidth * Nodes[i].Perpendicular;
				const FVector RightEdge = CenterCm - HalfTrackWidth * Nodes[i].Perpendicular;
				DrawDebugLine(GetWorld(), LeftEdge, RightEdge,
					FColor(60, 60, 60), true, DebugLineDuration, 0, 1.0f);

				// Left and right edges connected
				if (bClosedLoop || i < NumNodes - 1)
				{
					const int32 NextIdx = (i + 1) % NumNodes;
					const FVector NextCenterCm = Nodes[NextIdx].CenterPos * MToCm;
					const FVector NextLeft = NextCenterCm + HalfTrackWidth * Nodes[NextIdx].Perpendicular;
					const FVector NextRight = NextCenterCm - HalfTrackWidth * Nodes[NextIdx].Perpendicular;
					DrawDebugLine(GetWorld(), LeftEdge, NextLeft,
						FColor(40, 40, 40), true, DebugLineDuration, 0, 1.0f);
					DrawDebugLine(GetWorld(), RightEdge, NextRight,
						FColor(40, 40, 40), true, DebugLineDuration, 0, 1.0f);
				}
			}
		}
	}
}

TArray<FVector> ATurboRacingLineCalculator::GetRacingLinePositions() const
{
	return CachedPositions;
}

TArray<float> ATurboRacingLineCalculator::GetCurvatures() const
{
	return CachedCurvatures;
}
