#pragma once
#include "Character.h"
#include "Combat/EncounterInfo.h"
#include "glm/vec2.hpp"

class EnemyCharacter : public Character
{
public:
	EnemyCharacter();

	EnemyCharacter(EnemyInfo& info, glm::vec2 _renderPosition);

	void Render();

	void OnTurnStart(CombatStage* stage);
	void OnTurnUpdate(CombatStage* stage);

	void OnDeath();

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