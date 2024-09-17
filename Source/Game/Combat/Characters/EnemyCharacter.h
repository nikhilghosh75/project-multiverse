#pragma once
#include "Character.h"
#include "glm/vec2.hpp"

class EnemyCharacter : public Character
{
public:
	EnemyCharacter();
	EnemyCharacter(const EnemyCharacter* baseCharacter);
	EnemyCharacter(const rapidjson::Document& data);

	void Render();

	void OnTurnStart(CombatStage* stage);
	void OnTurnUpdate(CombatStage* stage);

	void OnDeath();

	int powerRating = 1;

protected:
	void SetFromJsonData(const rapidjson::Document& data);

private:
	enum class State
	{
		Deciding,
		Attacking,
		Cooldown
	};

	State currentState = State::Deciding;
	float timeLeftInState = 0.f;

	bool shouldRender = true;
};