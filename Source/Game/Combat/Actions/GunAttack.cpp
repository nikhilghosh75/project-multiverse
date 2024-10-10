#include "GunAttack.h"
#include "Core/Time.h"
#include "Combat/Characters/Character.h"
#include "UI/HUD/CombatHUD.h"
#include <cstdlib>
#include <iostream>

const float timeBetweenShots = 0.3f;

GunAttack::GunAttack()
{
	this->requiresTarget = true;
	this->instant = false;
	this->immediatelyEndsTurn = false;
}

GunAttack::GunAttack(std::string name, int damage, int variance, int shots, int shotsVariance, int cost)
	: name(name), baseDamage(damage), damageVariance(variance), baseShots(shots), shotsVariance(shotsVariance)
{
	this->cost = cost;

	this->requiresTarget = true;
	this->instant = false;
	this->immediatelyEndsTurn = false;
}

GunAttack::GunAttack(std::string name, int damage, int variance, int shots, int shotsVariance, int cost, std::string iconPath)
	: name(name), baseDamage(damage), damageVariance(variance), baseShots(shots), shotsVariance(shotsVariance)
{
	this->cost = cost;
	this->icon = new Texture(iconPath);

	this->requiresTarget = true;
	this->instant = false;
	this->immediatelyEndsTurn = false;
}

void GunAttack::StartExecuteOnTarget(CombatStage* combatStage, Character* executor, Character* character)
{
	Action::StartExecuteOnTarget(combatStage, executor, target);

	target = character;
	time = timeBetweenShots;
	currentShot = 0;

	executor->DeductActionPoints(cost);

	shotsThisAttack = baseShots + std::rand() % shotsVariance;
}

void GunAttack::UpdateExecute(CombatStage* combatStage, Character* executor)
{
	Action::UpdateExecute(combatStage, executor);

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
		"-" + std::to_string(baseShots + shotsVariance) + " Shots)";
}

void GunAttack::SetFromJson(const rapidjson::Value& data)
{
	name = data["name"].GetString();
	baseDamage = data["base_damage"].GetInt();
	damageVariance = data["damage_variance"].GetInt();
	baseShots = data["base_shots"].GetInt();
	shotsVariance = data["shots_variance"].GetInt();

	cost = data["cost"].GetInt();

	if (data.HasMember("icon"))
	{
		icon = new Texture(data["icon"].GetString());
	}
}

void GunAttack::ExecuteShot(CombatStage* combatStage, Character* executor)
{
	if (currentShot < shotsThisAttack)
	{
		currentShot++;

		int damage = baseDamage + std::rand() % damageVariance;
		int actualDamage = target->Damage(damage);

		glm::vec2 damageNumberPosition = target->baseScreenPosition + glm::vec2(0.05, -0.05);
		damageNumberPosition = damageNumberPosition * 2.f - glm::vec2(1.f, 1.f);
		CombatHUD::AddDamageNumber(FloatingDamageNumber(actualDamage, damageNumberPosition, 2.f));

		time += timeBetweenShots;
	}
	else
	{
		executor->EndAction(combatStage);
	}
}
