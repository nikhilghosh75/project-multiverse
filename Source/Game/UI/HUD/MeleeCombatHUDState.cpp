#include "MeleeCombatHUDState.h"
#include "EnemyTurnHUDState.h"
#include "MainCombatHUDState.h"
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
	Character* currentCharacter = stage->GetCurrentTurnCharacter();

	int meleeIndex = 0;
	for (int i = 0; i < currentCharacter->actions.size(); i++)
	{
		MeleeAttack* attack = dynamic_cast<MeleeAttack*>(currentCharacter->actions[i]);

		if (attack)
		{
			FontRenderer::Get()->AddText(attack->GetDisplayName(), glm::vec2(-0.8, 0.45 + meleeIndex * 0.12), 14);
			Button::Add(Rect(0.43 + meleeIndex * 0.12, 0.53 + meleeIndex * 0.12, -0.85, -0.55), [this, stage, attack]() 
				{
					this->StartExecuteAction(attack, stage);
				});
			meleeIndex++;
		}
	}
}

void MeleeCombatHUDState::OnTargetSelected(CombatStage* stage, Character* target)
{
	if (action != nullptr)
	{
		stage->GetCurrentTurnCharacter()->StartAction(action);
		action->StartExecuteOnTarget(stage, stage->GetCurrentTurnCharacter(), target);
		stage->GetCurrentTurnCharacter()->EndAction(stage);
	}
}

void MeleeCombatHUDState::OnTurnAdvanced(CombatStage* stage)
{
	if (stage->GetCurrentTurnCharacter()->type == CharacterType::Enemy)
	{
		CombatHUD::SetCurrentState(new EnemyTurnHUDState());
	}
	else
	{
		CombatHUD::SetCurrentState(new MainCombatHUDState());
	}
}

void MeleeCombatHUDState::OnActionEnded(CombatStage* stage, Character* character, Action* action)
{
	if (character->GetActionPoints() > 0)
	{
		CombatHUD::SetCurrentState(new MainCombatHUDState());
	}
}
