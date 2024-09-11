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
	PlayerCharacter* playerCharacter = stage->GetPlayerCharacter();

	int gunIndex = 0;
	for (int i = 0; i < playerCharacter->actions.size(); i++)
	{
		GunAttack* attack = dynamic_cast<GunAttack*>(playerCharacter->actions[i]);

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
	CombatHUD::SetCurrentState(new EnemyTurnHUDState());
}

void GunCombatHUDState::OnActionEnded(CombatStage* stage, Character* character, Action* action)
{
	if (character->GetActionPoints() > 0)
	{
		CombatHUD::SetCurrentState(new MainCombatHUDState());
	}
}
