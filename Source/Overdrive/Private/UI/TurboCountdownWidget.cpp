// Copyright Simon Kramer. All Rights Reserved.


#include "UI/TurboCountdownWidget.h"
#include "Framework/TurboRaceManager.h"
#include "Framework/TurboGameMode.h"
#include "CommonTextBlock.h"

void UTurboCountdownWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// clear preview text
	if (Txt_Countdown)
	{
		Txt_Countdown->SetText(FText::GetEmpty());
	}

	if (ATurboGameMode* GameMode = Cast<ATurboGameMode>(GetWorld()->GetAuthGameMode()))
	{
		if (UTurboRaceManager* RaceManager = GameMode->GetRaceManager())
		{
			RaceManager->OnCountdownUpdated.AddDynamic(this, &UTurboCountdownWidget::OnCountdownUpdated);
			RaceManager->OnRaceStarted.AddDynamic(this, &UTurboCountdownWidget::OnRaceStarted);
		}
	}
}

void UTurboCountdownWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RemoveTimerHandle);
	}

	if (ATurboGameMode* GameMode = Cast<ATurboGameMode>(GetWorld()->GetAuthGameMode()))
	{
		if (UTurboRaceManager* RaceManager = GameMode->GetRaceManager())
		{
			RaceManager->OnCountdownUpdated.RemoveDynamic(this, &UTurboCountdownWidget::OnCountdownUpdated);
			RaceManager->OnRaceStarted.RemoveDynamic(this, &UTurboCountdownWidget::OnRaceStarted);
		}
	}

	Super::NativeDestruct();
}

void UTurboCountdownWidget::OnCountdownUpdated(int32 Count)
{
	if (Txt_Countdown)
	{
		// Stop any in-progress animation
		if (Anim_Pop)
		{
			StopAnimation(Anim_Pop);
			Txt_Countdown->SetRenderScale(FVector2D(1.0f, 1.0f));
		}

		if (Count > 0)
		{
			Txt_Countdown->SetText(FText::AsNumber(Count));
		}
		else
		{
			Txt_Countdown->SetText(FText::FromString(TEXT("GO!")));
		}

		if (Anim_Pop)
		{
			PlayAnimation(Anim_Pop);
		}
	}
}

void UTurboCountdownWidget::OnRaceStarted()
{
	// Remove widget, but with a short delay, so that animation can finish
	// WARNING: If the delay is shorter than the animation, this will cause a crash
	// WARNING: If I pause and return to main menu during countdown, it will crash
	//FTimerHandle RemoveTimer;
	//GetWorld()->GetTimerManager().SetTimer(RemoveTimer, [this](){RemoveFromParent();}, 1.5f, false);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			RemoveTimerHandle,
			FTimerDelegate::CreateWeakLambda(this, [this]() { RemoveFromParent(); }),
			1.5f,
			false
		);
	}


}
