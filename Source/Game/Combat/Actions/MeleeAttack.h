#pragma once
#include "Action.h"
#include <string>

class MeleeAttack : public Action
{
public:
	MeleeAttack();
	MeleeAttack(std::string name, int damage, int variance, int cost);

	void StartExecuteOnTarget(CombatStage* combatStage, Character* executor, Character* character);

	int CalculateDamage();

	int GetLastDamageDealt() const;

	const std::string& GetName() const;

	std::string GetDisplayName() const;

	void SetFromJson(const rapidjson::Value& data);

private:
	int attackDamage;
	int variance;
	std::string name;

	int lastDamage;
};