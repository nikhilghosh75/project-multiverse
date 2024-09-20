#pragma once
#include "Character.h"

class CompanionCharacter : Character
{
public:
	CompanionCharacter();
	CompanionCharacter(const CompanionCharacter* baseCharacter);
	CompanionCharacter(const rapidjson::Document& data);

	void Render();

	void OnTurnStart(CombatStage* stage);

	void OnDeath();

protected:
	void SetFromJsonData(const rapidjson::Document& data);

};