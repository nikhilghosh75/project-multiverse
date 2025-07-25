#include "CombatHUD.h"
#include "Combat/CombatStage.h"
#include "FontRenderer.h"
#include "ImageRenderer.h"
#include "UI/HUD/MainCombatHUDState.h"
#include "VectorRenderer.h"

#include "Input.h"
#include "Window.h"
#include "ScreenCoordinate.h"
#include "Core/Time.h"

// TODO: Change to dynamic size
const float boxSize = 0.08f;
const float crosshairSize = 20.f;

const glm::vec2 healthBarOffset = glm::vec2(0, -0.08f);
const glm::vec2 characterNameOffset = glm::vec2(0, 0.10f);
const float healthBarWidth = 0.08f;

const float COMBAT_HUD_ORDER_START = 100.f;
const float HEALTHBAR_ORDER = 109.f;
const float CURRENT_TURN_TRIANGLE_ORDER = 110.f;
const float DAMAGE_NUMBER_ORDER = 111.f;
const float CROSSHAIR_ORDER = 112.f;

void CombatHUD::Initialize()
{
	currentState = new MainCombatHUDState();
	previousState = nullptr;

	crosshairTexture = new Texture("Data/Sprites/UI/Crosshair.png");
	triangleTexture = new Texture("Data/Sprites/UI/Red Triangle.png");
}

void CombatHUD::Cleanup()
{
	if (currentState != nullptr)
	{
		delete currentState;
	}

	delete crosshairTexture;
	delete triangleTexture;
}

void CombatHUD::Render(CombatStage* stage)
{
	if (previousState)
	{
		delete previousState;
		previousState = nullptr;
	}

	currentState->Render(stage);

	if (currentState->isSelectingTarget)
	{
		RenderTargetSelection(stage);
	}

	RenderDamageNumbers(stage);
	RenderCurrentTurnTriangle(stage);
	RenderCharacterHUDs(stage);
}

void CombatHUD::SetCurrentState(CombatHUDState* newState)
{
	previousState = currentState;
	currentState = newState;
}

CombatHUDState* CombatHUD::GetCurrentStage()
{
	return currentState;
}

void CombatHUD::AddDamageNumber(FloatingDamageNumber damageNumber)
{
	damageNumbers.push_back(damageNumber);
}

void CombatHUD::RenderTargetSelection(CombatStage* stage)
{
	FontRenderer::Get()->AddText("Select a target", glm::vec2(-0.2, -0.8), COMBAT_HUD_ORDER_START + 0.1f,  81);

	std::vector<glm::vec2>& enemyPositions = stage->GetEnemyPositions();
	std::vector<EnemyCharacter*> enemies = stage->GetEnemyCharacters();
	glm::vec2 mouseNormalizedPosition = Input::GetMouseNormalizedPosition();

	for (int i = 0; i < enemyPositions.size(); i++)
	{
		Rect enemyRect = Rect(enemyPositions[i].y - boxSize, enemyPositions[i].y + boxSize,
			enemyPositions[i].x - boxSize, enemyPositions[i].x + boxSize);

		if (enemyRect.IsPointInside(mouseNormalizedPosition.x, mouseNormalizedPosition.y))
		{
			FontRenderer::Get()->AddText(enemies[i]->name, glm::vec2(-0.1, -0.7), COMBAT_HUD_ORDER_START + 0.4f, 54);
			RenderCrosshair(enemyPositions[i]);

			if (Input::GetMouseButtonDown(MouseButton::Left))
			{
				currentState->OnTargetSelected(stage, enemies[i]);
			}
		}
	}
}

void CombatHUD::RenderCrosshair(glm::vec2 position)
{
	ScreenCoordinate crosshairCoordinate = ScreenCoordinate(glm::vec2(0, 0), position);
	Rect crosshairRect = ScreenCoordinate::CreateRect(crosshairCoordinate, glm::vec2(crosshairSize, crosshairSize), glm::vec2(0.5f, 0.5f));
	ImageRenderingOptions options;
	options.keepAspectRatio = true;
	ImageRenderer::Get()->AddImage(crosshairTexture, crosshairRect, CROSSHAIR_ORDER, options);
}

void CombatHUD::RenderDamageNumbers(CombatStage* stage)
{
	int indexToDelete = -1;
	for (int i = 0; i < damageNumbers.size(); i++)
	{
		if (damageNumbers[i].startTime + damageNumbers[i].duration < Time::GetTime())
		{
			indexToDelete = i;
			continue;
		}

		FontRenderer::Get()->AddText(std::to_string(damageNumbers[i].damage), damageNumbers[i].startingPosition, DAMAGE_NUMBER_ORDER, 108);
	}

	if (indexToDelete != -1)
	{
		damageNumbers.erase(damageNumbers.begin() + indexToDelete);
	}
}

void CombatHUD::RenderCurrentTurnTriangle(CombatStage* stage)
{
	Character* character = stage->GetCurrentTurnCharacter();
	ScreenCoordinate coordinate = ScreenCoordinate(glm::vec2(0, -65), character->baseScreenPosition);
	Rect rect = ScreenCoordinate::CreateRect(coordinate, glm::vec2(15, 15), glm::vec2(0.5, 0.5));
	ImageRenderer::Get()->AddImage(triangleTexture, rect, CURRENT_TURN_TRIANGLE_ORDER);
}

void CombatHUD::RenderCharacterHUDs(CombatStage* stage)
{
	VectorPainter painter(HEALTHBAR_ORDER);

	RenderCharacterHUD(stage, stage->GetPlayerCharacter(), &painter);

	for (Character* companion : stage->GetCompanionCharacters())
	{
		if (!companion->IsDead())
		{
			RenderCharacterHUD(stage, companion, &painter);
		}
	}

	for (Character* enemy : stage->GetEnemyCharacters())
	{
		if (!enemy->IsDead())
		{
			RenderCharacterHUD(stage, enemy, &painter);
		}
	}

	VectorRenderer::Get()->SubmitPainter(painter);
}

void CombatHUD::RenderCharacterHUD(CombatStage* stage, Character* character, VectorPainter* painter)
{
	glm::vec2 textPosition = (character->baseScreenPosition + characterNameOffset) * 2.f - glm::vec2(1.f, 1.f);
	FontRenderer::Get()->AddText(character->name, textPosition, COMBAT_HUD_ORDER_START + 0.7f, 48);

	float healthPercent = (float)character->GetHealth() / (float)character->GetMaxHealth();
	glm::vec2 basePosition = character->baseScreenPosition + healthBarOffset;

	painter->SetFillColor(Color((uint8_t)130, 40, 40, 255));
	painter->BeginPath();
	painter->PointTo(basePosition + glm::vec2(-healthBarWidth / 2, 0));
	painter->LineTo(basePosition + glm::vec2(-healthBarWidth / 2, -0.01));
	painter->LineTo(basePosition + glm::vec2(healthBarWidth / 2, -0.01));
	painter->LineTo(basePosition + glm::vec2(healthBarWidth / 2, 0));
	painter->LineTo(basePosition + glm::vec2(-healthBarWidth / 2, 0));
	painter->ClosePath();

	painter->SetFillColor(Color((uint8_t)240, 10, 10, 255));
	painter->BeginPath();
	painter->PointTo(basePosition + glm::vec2(-healthBarWidth / 2, 0));
	painter->LineTo(basePosition + glm::vec2(-healthBarWidth / 2, -0.01));
	painter->LineTo(basePosition + glm::vec2(-healthBarWidth / 2 + healthBarWidth * healthPercent, -0.01));
	painter->LineTo(basePosition + glm::vec2(-healthBarWidth / 2 + healthBarWidth * healthPercent, 0));
	painter->LineTo(basePosition + glm::vec2(-healthBarWidth / 2, 0));
	painter->ClosePath();

	if (character->type == CharacterType::Player || character->type == CharacterType::Companion)
	{
		FontRenderer::Get()->AddText(std::to_string(character->GetActionPoints()) + " AP", textPosition + glm::vec2(0, 0.04), COMBAT_HUD_ORDER_START + 0.7f, 72);
	}
}

CombatHUDState::CombatHUDState()
	: action(nullptr), isSelectingTarget(false)
{
}

void CombatHUDState::StartExecuteAction(std::shared_ptr<Action> action, CombatStage* combatStage)
{
	this->action = action;

	if (action->RequiresTarget())
	{
		isSelectingTarget = true;
	}
	else
	{
		combatStage->GetCurrentTurnCharacter()->StartAction(action);
		action->StartExecute(combatStage, combatStage->GetCurrentTurnCharacter());

		if (action->Instant())
		{
			combatStage->GetCurrentTurnCharacter()->EndAction(combatStage);
		}
	}
}

FloatingDamageNumber::FloatingDamageNumber(int damage, glm::vec2 startingPosition, float duration)
	: damage(damage), startingPosition(startingPosition), duration(duration)
{
	startTime = Time::GetTime();
}
