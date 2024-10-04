#include "MeleeAttack.h"
#include "Combat/CombatStage.h"
#include "Combat/Characters/Character.h"
#include "UI/HUD/CombatHUD.h"
#include <cstdlib>

MeleeAttack::MeleeAttack()
{
	this->requiresTarget = true;
	this->immediatelyEndsTurn = false;
	this->instant = true;
}

MeleeAttack::MeleeAttack(std::string name, int damage, int variance, int cost)
	: name(name), attackDamage(damage), variance(variance), lastDamage(0)
{
	this->cost = cost;

	this->requiresTarget = true;
	this->instant = true;
	this->immediatelyEndsTurn = false;
}

MeleeAttack::MeleeAttack(std::string name, int damage, int variance, int cost, std::string iconPath)
	: name(name), attackDamage(damage), variance(variance), lastDamage(0)
{
	this->cost = cost;
	this->icon = new Texture(iconPath);

	this->requiresTarget = true;
	this->instant = true;
	this->immediatelyEndsTurn = false;
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

void MeleeAttack::SetFromJson(const rapidjson::Value& data)
{
	name = data["name"].GetString();
	cost = data["cost"].GetInt();
	attackDamage = data["base_damage"].GetInt();
	variance = data["variance"].GetInt();

	if (data.HasMember("icon"))
	{
		icon = new Texture(data["icon"].GetString());
	}
}
