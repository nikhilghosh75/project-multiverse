#include "GuardCombatHUDState.h"
#include "Combat/CombatStage.h"
#include "Combat/Actions/GuardAction.h"
#include "Combat/Characters/PlayerCharacter.h"
#include "EnemyTurnHUDState.h"
#include "FontRenderer.h"
#include "UI/Button.h"

GuardCombatHUDState::GuardCombatHUDState()
{

}

void GuardCombatHUDState::Render(CombatStage* stage)
{
	PlayerCharacter* playerCharacter = stage->GetPlayerCharacter();

	int guardIndex = 0;
	for (int i = 0; i < playerCharacter->actions.size(); i++)
	{
		GuardAction* guard = dynamic_cast<GuardAction*>(playerCharacter->actions[i]);

		if (guard)
		{
			FontRenderer::Get()->AddText(guard->GetDisplayName(), glm::vec2(-0.8, 0.45 + guardIndex * 0.12), 14);
			Button::Add(Rect(0.43 + guardIndex * 0.12, 0.53 + guardIndex * 0.12, -0.85, -0.55), [this, stage, guard]()
				{
					stage->GetCurrentTurnCharacter()->StartAction(guard);
					this->StartExecuteAction(guard, stage);
					stage->GetCurrentTurnCharacter()->EndAction(stage);
				});
			guardIndex++;
		}
	}
}

void GuardCombatHUDState::OnTurnAdvanced(CombatStage* stage)
{
	CombatHUD::SetCurrentState(new EnemyTurnHUDState());
}