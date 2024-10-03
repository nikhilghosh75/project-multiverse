#include "MainCombatHUDState.h"
#include "Combat/CombatStage.h"
#include "Combat/Actions/PassAction.h"
#include "FontRenderer.h"
#include "ImageRenderer.h"
#include "VectorRenderer.h"
#include "ScreenCoordinate.h"
#include "UI/Button.h"
#include "EnemyTurnHUDState.h"
#include "GuardCombatHUDState.h"
#include "GunCombatHUDState.h"
#include "MeleeCombatHUDState.h"

#include <iostream>

const glm::vec2 startPosition = glm::vec2(0.15, 0.75);

MainCombatHUDState::MainCombatHUDState()
{
	noIconTexture = new Texture("Data/Sprites/UI/Icons/Icon Null.png");
}

void MainCombatHUDState::Render(CombatStage* stage)
{
	glm::vec2 position = startPosition;

	ImageRenderingOptions options;
	options.keepAspectRatio = true;

	VectorPainter painter;
	painter.SetFillColor(Color(1.0f, 0.0f, 0.0f, 1.0f));

	std::vector<Action*>& actions = stage->GetPlayerCharacter()->actions;
	for (int i = 0; i < actions.size(); i++)
	{
		ScreenCoordinate coordinate = ScreenCoordinate(glm::vec2(10, 10), position);
		Rect rect = ScreenCoordinate::CreateRect(coordinate, glm::vec2(50, 83.3), glm::vec2(0.5, 0.5));
		Texture* texture = actions[i]->GetTexture() == nullptr ? noIconTexture : actions[i]->GetTexture();

		ImageRenderer::Get()->AddImage(texture, rect, options);

		glm::vec2 bottomRight = glm::vec2(rect.right, rect.bottom);
		painter.DrawRegularPolygon(bottomRight, 6, 0.02f);

		position += glm::vec2(0.1, 0);
	}

	VectorRenderer::Get()->SubmitPainter(painter);

	position = startPosition;
	for (int i = 0; i < actions.size(); i++)
	{
		ScreenCoordinate coordinate = ScreenCoordinate(glm::vec2(10, 10), position);
		Rect rect = ScreenCoordinate::CreateRect(coordinate, glm::vec2(50, 83.3), glm::vec2(0.5, 0.5));

		glm::vec2 bottomRight = glm::vec2(rect.right, rect.bottom);
		bottomRight = bottomRight * 2.f - glm::vec2(1, 1);
		FontRenderer::Get()->AddText(std::to_string(actions[i]->GetCost()), bottomRight + glm::vec2(-0.006, 0.012), 24);

		position += glm::vec2(0.1, 0);
	}
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
	if (stage->GetCurrentTurnCharacter()->type == CharacterType::Enemy)
	{
		CombatHUD::SetCurrentState(new EnemyTurnHUDState());
	}
	else
	{
		CombatHUD::SetCurrentState(new MainCombatHUDState());
	}
}