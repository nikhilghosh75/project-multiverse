#include "MainCombatHUDState.h"
#include "Combat/CombatStage.h"
#include "Combat/Actions/PassAction.h"
#include "DebugRenderer.h"
#include "FontRenderer.h"
#include "ImageRenderer.h"
#include "VectorRenderer.h"
#include "ScreenCoordinate.h"
#include "UI/Button.h"
#include "EnemyTurnHUDState.h"

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

	std::vector<std::shared_ptr<Action>>& actions = stage->GetCurrentTurnCharacter()->actions;
	for (int i = 0; i < actions.size(); i++)
	{
		ScreenCoordinate coordinate = ScreenCoordinate(glm::vec2(10, 10), position);
		Rect rect = ScreenCoordinate::CreateRect(coordinate, glm::vec2(50, 83.3), glm::vec2(0.5, 0.5));
		Texture* texture = actions[i]->GetTexture() == nullptr ? noIconTexture : actions[i]->GetTexture();

		ImageRenderingResult result = ImageRenderer::Get()->AddImage(texture, rect, options);

		glm::vec2 bottomRight = glm::vec2(rect.right, rect.bottom);
		painter.DrawRegularPolygon(bottomRight, 6, 0.026f);

		bool hovered = false;
		Rect renderingRect = ScreenCoordinate::ConvertRectBetweenSpaces(result.finalRect, ScreenSpace::Screen, ScreenSpace::Rendering);
		Button::Add(renderingRect, 
			[this, stage, &actions, i]() { this->StartExecuteAction(actions[i], stage); }, 
			[&hovered]() { hovered = true; }
		);

		if (hovered)
		{
			glm::vec2 textPosition = ScreenCoordinate::ConvertPointBetweenSpace(position + glm::vec2(0, 0.15), ScreenSpace::Screen, ScreenSpace::Rendering);
			FontRenderer::Get()->AddText(actions[i]->GetDisplayName(), textPosition, 48);
		}

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
		FontRenderer::Get()->AddText(std::to_string(actions[i]->GetCost()), bottomRight + glm::vec2(-0.014, 0.034), 72);

		Rect screenRect = ScreenCoordinate::ConvertRectBetweenSpaces(rect, ScreenSpace::Screen, ScreenSpace::Rendering);
		// DebugRenderer::Get()->AddBox(screenRect);

		position += glm::vec2(0.1, 0);
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

void MainCombatHUDState::OnTargetSelected(CombatStage* stage, Character* character)
{
	stage->GetCurrentTurnCharacter()->StartAction(action);
	action->StartExecuteOnTarget(stage, stage->GetCurrentTurnCharacter(), character);

	if (action->Instant())
	{
		stage->GetCurrentTurnCharacter()->EndAction(stage);
	}
}

