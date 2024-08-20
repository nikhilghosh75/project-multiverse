#pragma once
#include "Action.h"
#include <string>

class MeleeAttack : public Action
{
	int attackDamage;
	int variance;
	std::string name;

public:
	MeleeAttack(std::string name, int damage, int variance, int cost);

	void ExecuteOnTarget(CombatStage* combatStage, Character* character);

	int CalculateDamage();

	int GetLastDamageDealt() const;

	const std::string& GetName() const;

	std::string GetDisplayName() const;

private:
	int lastDamage;
};