// Copyright Simon Kramer. All Rights Reserved.


#include "UI/TurboCountdownWidget.h"
#include "Framework/TurboRaceManager.h"
#include "Framework/TurboGameMode.h"
#include "CommonTextBlock.h"

void UTurboCountdownWidget::NativeConstruct()
{
	Super::NativeConstruct();

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
	if (Anim_Pop && Txt_Countdown)
	{
		FWidgetAnimationDynamicEvent FinishedDelegate;
		FinishedDelegate.BindDynamic(this, &UTurboCountdownWidget::OnRemoveAnimFinished);

		BindToAnimationFinished(Anim_Pop, FinishedDelegate);

		if (!IsAnimationPlaying(Anim_Pop))
		{
			RemoveFromParent();
			return;
		}
	}
	else
	{
		RemoveFromParent();
	}
}

void UTurboCountdownWidget::OnRemoveAnimFinished()
{
	RemoveFromParent();
}
