#include "MeleeCombatHUDState.h"
#include "EnemyTurnHUDState.h"
#include "Combat/CombatStage.h"
#include "Combat/Actions/MeleeAttack.h"
#include "Combat/Characters/PlayerCharacter.h"
#include "FontRenderer.h"
#include "UI/Button.h"

MeleeCombatHUDState::MeleeCombatHUDState()
	: selectedAttack(nullptr)
{
}

void MeleeCombatHUDState::Render(CombatStage* stage)
{
	PlayerCharacter* playerCharacter = stage->GetPlayerCharacter();

	int numMeleeActions = 0;
	for (int i = 0; i < playerCharacter->actions.size(); i++)
	{
		MeleeAttack* attack = dynamic_cast<MeleeAttack*>(playerCharacter->actions[i]);

		if (attack)
		{
			FontRenderer::Get()->AddText(attack->GetDisplayName(), glm::vec2(-0.8, 0.45 + i * 0.12), 14);
			Button::Add(Rect(0.43 + i * 0.12, 0.53 + i * 0.12, -0.85, -0.55), [this, stage, attack]() 
				{
					this->StartExecuteAction(attack, stage);
				});
		}
	}
}

void MeleeCombatHUDState::OnTargetSelected(CombatStage* stage, Character* character)
{
	if (action != nullptr)
	{
		action->ExecuteOnTarget(stage, stage->GetCurrentTurnCharacter(), character);
		if (stage->GetPlayerCharacter()->GetActionPoints() == 0)
		{
			stage->AdvanceTurn();
			CombatHUD::SetCurrentState(new EnemyTurnHUDState());
		}
	}
}
