#include "MeleeAttack.h"
#include "Combat/CombatStage.h"
#include "Combat/Characters/Character.h"
#include <cstdlib>

MeleeAttack::MeleeAttack(std::string name, int damage, int variance, int cost)
	: name(name), attackDamage(damage), variance(variance), lastDamage(0)
{
	this->cost = cost;

	this->requiresTarget = true;
}

void MeleeAttack::ExecuteOnTarget(CombatStage* combatStage, Character* character)
{
	
}

int MeleeAttack::CalculateDamage()
{
	lastDamage = attackDamage + std::rand() % variance;

	return lastDamage;
}

int MeleeAttack::GetLastDamageDealt() const
{
	return lastDamage;
}

const std::string& MeleeAttack::GetName() const
{
	return name;
}

std::string MeleeAttack::GetDisplayName() const
{
	return name + " (" + std::to_string(attackDamage) + "-"
		+ std::to_string(attackDamage + variance) + " DMG)";
}
