#include "Inventory/UI/AGSDComboGuideWidget.h"
#include "Components/TextBlock.h"

void UAGSDComboGuideWidget::UpdateComboGuide(const TArray<FComboGuideEntry>& GuideEntries)
{
	// 첫 번째 가이드 항목
	if (GuideEntries.IsValidIndex(0))
	{
		if (ActionInputText)
		{
			ActionInputText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			ActionInputText->SetText(GuideEntries[0].InputText);
		}
		if (AttackNameText)
		{
			AttackNameText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			AttackNameText->SetText(GuideEntries[0].AttackNameText);
		}
	}
	else
	{
		if (ActionInputText) ActionInputText->SetVisibility(ESlateVisibility::Collapsed);
		if (AttackNameText) AttackNameText->SetVisibility(ESlateVisibility::Collapsed);
	}

	// 두 번째 가이드 항목
	if (GuideEntries.IsValidIndex(1))
	{
		if (SecondaryActionInputText)
		{
			SecondaryActionInputText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			SecondaryActionInputText->SetText(GuideEntries[1].InputText);
		}
		if (SecondaryAttackNameText)
		{
			SecondaryAttackNameText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			SecondaryAttackNameText->SetText(GuideEntries[1].AttackNameText);
		}
	}
	else
	{
		if (SecondaryActionInputText) SecondaryActionInputText->SetVisibility(ESlateVisibility::Collapsed);
		if (SecondaryAttackNameText) SecondaryAttackNameText->SetVisibility(ESlateVisibility::Collapsed);
	}
}
