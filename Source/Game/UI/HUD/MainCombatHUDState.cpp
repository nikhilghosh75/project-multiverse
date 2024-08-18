#include "MainCombatHUDState.h"
#include "Combat/CombatStage.h"
#include "FontRenderer.h"
#include "UI/Button.h"

#include <iostream>

MainCombatHUDState::MainCombatHUDState()
{
}

void MainCombatHUDState::Render(CombatStage* stage)
{
	// TODO: Add Localization
	
	FontRenderer::Get()->AddText("Melee", glm::vec2(-0.8, 0.45), 14);
	Button::Add(Rect(0.43, 0.53, -0.85, -0.55), [this]() { this->OnMeleeButtonClicked(); });

	FontRenderer::Get()->AddText("Gun", glm::vec2(-0.8, 0.57), 14);
	Button::Add(Rect(0.55, 0.65, -0.85, -0.55), [this]() { this->OnGunButtonClicked(); });

	FontRenderer::Get()->AddText("Guard", glm::vec2(-0.8, 0.69), 14);
	Button::Add(Rect(0.67, 0.77, -0.85, -0.55), [this]() { this->OnGuardButtonClicked(); });

	FontRenderer::Get()->AddText("Pass", glm::vec2(-0.8, 0.81), 14);
	Button::Add(Rect(0.79, 0.89, -0.85, -0.55), [this]() { this->OnPassButtonClicked(); });
}

void MainCombatHUDState::OnMeleeButtonClicked()
{
	std::cout << "Melee Clicked" << std::endl;
}

void MainCombatHUDState::OnGunButtonClicked()
{
	std::cout << "Gun Clicked" << std::endl;
}

void MainCombatHUDState::OnGuardButtonClicked()
{
	std::cout << "Guard Clicked" << std::endl;
}

void MainCombatHUDState::OnPassButtonClicked()
{
	std::cout << "Pass Clicked" << std::endl;
}
