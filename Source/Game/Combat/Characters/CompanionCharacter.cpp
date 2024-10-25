#include "CompanionCharacter.h"
#include "ScreenCoordinate.h"
#include "ImageRenderer.h"
#include "Core/RunManager.h"
#include "Combat/Actions/PassAction.h"

// TODO: Change when multiple companions become possible
glm::vec2 companionPosition = glm::vec2(0.1f, 0.2f);

CompanionCharacter::CompanionCharacter()
{
	type = CharacterType::Companion;
}

CompanionCharacter::CompanionCharacter(const CompanionCharacter* baseCharacter)
{
	type = CharacterType::Companion;
	
	for (int i = 0; i < baseCharacter->actions.size(); i++)
	{
		actions.push_back(baseCharacter->actions[i]);
	}

	actionPointsPerTurn = baseCharacter->actionPointsPerTurn;
	name = baseCharacter->name;
	maxHealth = baseCharacter->maxHealth;
	health = baseCharacter->health;

	texture = baseCharacter->texture;

	baseScreenPosition = companionPosition;
	screenOffset = glm::vec2(0, 0);
}

CompanionCharacter::CompanionCharacter(const rapidjson::Document& data)
{
	type = CharacterType::Companion;
	baseScreenPosition = companionPosition;
	screenOffset = glm::vec2(0, 0);
	SetFromJsonData(data);
}

void CompanionCharacter::Render()
{
	if (!shouldRender)
	{
		return;
	}

	ScreenCoordinate companionScreenPosition = ScreenCoordinate(glm::vec2(0, 0), baseScreenPosition + screenOffset);
	Rect rect = ScreenCoordinate::CreateRect(companionScreenPosition, glm::vec2(80, 120), glm::vec2(0.5, 0.5));

	ImageRenderingOptions options;
	options.keepAspectRatio = true;
	ImageRenderer::Get()->AddImage(texture, rect, options);
}

void CompanionCharacter::OnTurnStart(CombatStage* stage)
{

}

void CompanionCharacter::OnDeath()
{
	shouldRender = false;
	RunManager::GetPlayerState()->RemoveCompanion(this);
}

void CompanionCharacter::SetFromJsonData(const rapidjson::Document& data)
{
	name = data["name"].GetString();
	texture = new Texture(data["sprite"].GetString());
	type = CharacterType::Companion;

	health = data["starting_health"].GetInt();
	maxHealth = data["starting_health"].GetInt();

	actionPointsPerTurn = data["ap_per_turn"].GetInt();

	for (const auto& it : data["actions"].GetArray())
	{
		actions.push_back(Action::CreateFromJson(it));
	}
	actions.push_back(std::make_shared<PassAction>());
}
