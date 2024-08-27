#pragma once
#include "Action.h"
#include <string>

class MeleeAttack : public Action
{
public:
	MeleeAttack(std::string name, int damage, int variance, int cost);

	void ExecuteOnTarget(CombatStage* combatStage, Character* executor, Character* character);

	int CalculateDamage();

	int GetLastDamageDealt() const;

	const std::string& GetName() const;

	std::string GetDisplayName() const;

private:
	int attackDamage;
	int variance;
	std::string name;

	int lastDamage;
};