#include "GunAttack.h"
#include "Core/Time.h"
#include "Combat/Characters/Character.h"
#include "UI/HUD/CombatHUD.h"
#include <cstdlib>

const float timeBetweenShots = 0.4f;

GunAttack::GunAttack(std::string name, int damage, int variance, int shots, int shotsVariance, int cost)
	: name(name), baseDamage(damage), damageVariance(variance), baseShots(shots), shotsVariance(shotsVariance)
{
	this->cost = cost;

	this->requiresTarget = true;
	this->instant = false;
}

void GunAttack::StartExecuteOnTarget(CombatStage* combatStage, Character* executor, Character* character)
{
	target = character;
	time = timeBetweenShots;
	currentShot = 0;

	shotsThisAttack = baseShots + std::rand() % shotsVariance;
}

void GunAttack::UpdateExecute(CombatStage* combatStage, Character* executor)
{
	time -= Time::GetDeltaTime();

	if (time < 0)
	{
		ExecuteShot(combatStage, executor);
	}
}

std::string GunAttack::GetDisplayName() const
{
	return name + "(" + std::to_string(baseDamage) + "-" +
		std::to_string(baseDamage + damageVariance) + " DMG per shot, " + std::to_string(baseShots) +
		"-" + std::to_string(baseShots + shotsVariance) + " Shots, " + std::to_string(cost) + " AP)";
}

void GunAttack::ExecuteShot(CombatStage* combatStage, Character* executor)
{
	if (currentShot < shotsThisAttack)
	{
		currentShot++;

		int damage = baseDamage + std::rand() % damageVariance;
		int actualDamage = target->Damage(damage);

		glm::vec2 damageNumberPosition = target->screenPosition + glm::vec2(0.05, -0.05);
		damageNumberPosition = damageNumberPosition * 2.f - glm::vec2(1.f, 1.f);
		CombatHUD::AddDamageNumber(FloatingDamageNumber(actualDamage, damageNumberPosition, 2.f));

		time += timeBetweenShots;
	}
	else
	{
		executor->EndAction(combatStage);
	}
}
