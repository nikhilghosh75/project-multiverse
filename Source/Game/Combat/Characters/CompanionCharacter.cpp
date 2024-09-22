#include "CompanionCharacter.h"
#include "ScreenCoordinate.h"
#include "ImageRenderer.h"

CompanionCharacter::CompanionCharacter()
{
	type = CharacterType::Companion;
}

CompanionCharacter::CompanionCharacter(const CompanionCharacter* baseCharacter)
{
	type = CharacterType::Companion;
	actions = baseCharacter->actions;
	actionPointsPerTurn = baseCharacter->actionPointsPerTurn;
	name = baseCharacter->name;
	maxHealth = baseCharacter->maxHealth;
	health = baseCharacter->health;

	texture = baseCharacter->texture;
}

CompanionCharacter::CompanionCharacter(const rapidjson::Document& data)
{
	type = CharacterType::Companion;
	SetFromJsonData(data);
}

void CompanionCharacter::Render()
{
	ScreenCoordinate enemyPosition = ScreenCoordinate(glm::vec2(0, 0), screenPosition);
	Rect rect = ScreenCoordinate::CreateRect(enemyPosition, glm::vec2(80, 120), glm::vec2(0.5, 0.5));

	ImageRenderingOptions options;
	options.keepAspectRatio = true;
	ImageRenderer::Get()->AddImage(texture, rect, options);
}

void CompanionCharacter::OnTurnStart(CombatStage* stage)
{

}

void CompanionCharacter::OnDeath()
{
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
}
