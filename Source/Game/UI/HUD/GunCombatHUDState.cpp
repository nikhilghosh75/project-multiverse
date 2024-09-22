#include "GunCombatHUDState.h"
#include "EnemyTurnHUDState.h"
#include "MainCombatHUDState.h"
#include "Combat/CombatStage.h"
#include "Combat/Actions/GunAttack.h"
#include "Combat/Characters/PlayerCharacter.h"
#include "FontRenderer.h"
#include "UI/Button.h"

GunCombatHUDState::GunCombatHUDState()
	: selectedAttack(nullptr)
{
}

void GunCombatHUDState::Render(CombatStage* stage)
{
	Character* currentCharacter = stage->GetCurrentTurnCharacter();

	int gunIndex = 0;
	for (int i = 0; i < currentCharacter->actions.size(); i++)
	{
		GunAttack* attack = dynamic_cast<GunAttack*>(currentCharacter->actions[i]);

		if (attack)
		{
			FontRenderer::Get()->AddText(attack->GetDisplayName(), glm::vec2(-0.8, 0.45 + gunIndex * 0.12), 14);
			Button::Add(Rect(0.43 + gunIndex * 0.12, 0.53 + gunIndex * 0.12, -0.85, -0.55), [this, stage, attack]()
				{
					this->StartExecuteAction(attack, stage);
				});
			gunIndex++;
		}
	}
}

void GunCombatHUDState::OnTargetSelected(CombatStage* stage, Character* character)
{
	if (action)
	{
		stage->GetCurrentTurnCharacter()->StartAction(action);
		action->StartExecuteOnTarget(stage, stage->GetCurrentTurnCharacter(), character);
	}
}

void GunCombatHUDState::OnTurnAdvanced(CombatStage* stage)
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

void GunCombatHUDState::OnActionEnded(CombatStage* stage, Character* character, Action* action)
{
	if (character->GetActionPoints() > 0)
	{
		CombatHUD::SetCurrentState(new MainCombatHUDState());
	}
}
