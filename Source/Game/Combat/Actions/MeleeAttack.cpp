#include "MeleeAttack.h"
#include "Combat/CombatStage.h"
#include "Combat/Characters/Character.h"
#include "UI/HUD/CombatHUD.h"
#include <cstdlib>

MeleeAttack::MeleeAttack(std::string name, int damage, int variance, int cost)
	: name(name), attackDamage(damage), variance(variance), lastDamage(0)
{
	this->cost = cost;

	this->requiresTarget = true;
	this->instant = true;
}

void MeleeAttack::StartExecuteOnTarget(CombatStage* combatStage, Character* executor, Character* target)
{
	CalculateDamage();

	int actualDamage = target->Damage(lastDamage);
	executor->DeductActionPoints(cost);

	glm::vec2 damageNumberPosition = target->screenPosition + glm::vec2(0.05, -0.05);
	damageNumberPosition = damageNumberPosition * 2.f - glm::vec2(1.f, 1.f);
	CombatHUD::AddDamageNumber(FloatingDamageNumber(actualDamage, damageNumberPosition, 2.f));
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
