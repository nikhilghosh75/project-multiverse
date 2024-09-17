#include "EnemyCharacter.h"
#include "ScreenCoordinate.h"
#include "ImageRenderer.h"
#include "Core/Time.h"
#include "Combat/CombatStage.h"
#include <iostream>

const float decisionTime = 0.5f;
const float cooldownTime = 2.5f;

EnemyCharacter::EnemyCharacter()
{
	type = CharacterType::Enemy;
}

EnemyCharacter::EnemyCharacter(const EnemyCharacter* baseCharacter)
{
	type = CharacterType::Enemy;
	actions = baseCharacter->actions;
	actionPointsPerTurn = baseCharacter->actionPointsPerTurn;
	name = baseCharacter->name;
	maxHealth = baseCharacter->maxHealth;
	health = baseCharacter->health;

	texture = baseCharacter->texture;
}

EnemyCharacter::EnemyCharacter(const rapidjson::Document& data)
{
	type = CharacterType::Enemy;
	SetFromJsonData(data);
}

void EnemyCharacter::Render()
{
	if (!shouldRender)
	{
		return;
	}

	ScreenCoordinate enemyPosition = ScreenCoordinate(glm::vec2(0, 0), screenPosition);
	Rect rect = ScreenCoordinate::CreateRect(enemyPosition, glm::vec2(80, 120), glm::vec2(0.5, 0.5));

	ImageRenderingOptions options;
	options.keepAspectRatio = true;
	ImageRenderer::Get()->AddImage(texture, rect, options);
}

void EnemyCharacter::OnTurnStart(CombatStage* stage)
{
	Character::OnTurnStart(stage);

	currentState = State::Deciding;
	timeLeftInState = decisionTime;
}

void EnemyCharacter::OnTurnUpdate(CombatStage* stage)
{
	timeLeftInState -= Time::GetDeltaTime();

	if (currentState == State::Cooldown)
	{
		actions[0]->UpdateExecute(stage, this);
	}

	if (timeLeftInState < 0.f)
	{
		switch (currentState)
		{
		case State::Attacking:
			actions[0]->StartExecuteOnTarget(stage, this, stage->GetPlayerCharacter());
			currentState = State::Cooldown;
			timeLeftInState = cooldownTime;
			break;
		case State::Cooldown:
			if (timeLeftInState < 0)
			{
				stage->AdvanceTurn();
			}
			break;
		case State::Deciding:
			if (timeLeftInState < 0)
			{
				currentState = State::Attacking;
			}
		}
	}
}

void EnemyCharacter::OnDeath()
{
	shouldRender = false;
}

void EnemyCharacter::SetFromJsonData(const rapidjson::Document& data)
{
	name = data["name"].GetString();
	texture = new Texture(data["sprite"].GetString());
	type = CharacterType::Enemy;

	powerRating = data["power_rating"].GetInt();

	health = data["starting_health"].GetInt();
	maxHealth = data["starting_health"].GetInt();

	actionPointsPerTurn = data["ap_per_turn"].GetInt();
	
	for (const auto& it : data["actions"].GetArray())
	{
		actions.push_back(Action::CreateFromJson(it));
	}
}
