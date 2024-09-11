#include "MainCombatHUDState.h"
#include "Combat/CombatStage.h"
#include "Combat/Actions/PassAction.h"
#include "FontRenderer.h"
#include "UI/Button.h"
#include "EnemyTurnHUDState.h"
#include "GuardCombatHUDState.h"
#include "GunCombatHUDState.h"
#include "MeleeCombatHUDState.h"

#include <iostream>

MainCombatHUDState::MainCombatHUDState()
{
}

void MainCombatHUDState::Render(CombatStage* stage)
{
	// TODO: Add Localization
	
	FontRenderer::Get()->AddText("Melee", glm::vec2(-0.8, 0.45), 14);
	Button::Add(Rect(0.43, 0.53, -0.85, -0.35), [this]() { this->OnMeleeButtonClicked(); });

	FontRenderer::Get()->AddText("Gun", glm::vec2(-0.8, 0.57), 14);
	Button::Add(Rect(0.55, 0.65, -0.85, -0.35), [this]() { this->OnGunButtonClicked(); });

	FontRenderer::Get()->AddText("Guard", glm::vec2(-0.8, 0.69), 14);
	Button::Add(Rect(0.67, 0.77, -0.85, -0.35), [this]() { this->OnGuardButtonClicked(); });

	FontRenderer::Get()->AddText("Pass", glm::vec2(-0.8, 0.81), 14);
	Button::Add(Rect(0.77, 0.89, -0.85, -0.35), [this, stage]() { this->OnPassButtonClicked(stage); });
}

void MainCombatHUDState::OnMeleeButtonClicked()
{
	CombatHUD::SetCurrentState(new MeleeCombatHUDState());
}

void MainCombatHUDState::OnGunButtonClicked()
{
	CombatHUD::SetCurrentState(new GunCombatHUDState());
}

void MainCombatHUDState::OnGuardButtonClicked()
{
	CombatHUD::SetCurrentState(new GuardCombatHUDState());
}

void MainCombatHUDState::OnPassButtonClicked(CombatStage* stage)
{
	PassAction* passAction = stage->GetCurrentTurnCharacter()->FindFirstActionOfType<PassAction>();

	if (passAction)
	{
		stage->GetCurrentTurnCharacter()->StartAction(passAction);
		passAction->StartExecute(stage, stage->GetCurrentTurnCharacter());
		stage->GetCurrentTurnCharacter()->EndAction(stage);
	}
}

void MainCombatHUDState::OnTurnAdvanced(CombatStage* stage)
{
	CombatHUD::SetCurrentState(new EnemyTurnHUDState());
}