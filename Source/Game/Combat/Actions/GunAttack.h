#pragma once
#include "Action.h"
#include <string>

class GunAttack : public Action
{
public:
	GunAttack();

	GunAttack(std::string name, int damage, int variance, int shots, int shotsVariance, int cost);

	void StartExecuteOnTarget(CombatStage* combatStage, Character* executor, Character* character);

	void UpdateExecute(CombatStage* combatStage, Character* executor);

	std::string GetDisplayName() const;

	void SetFromJson(const rapidjson::Value& data);

private:
	int baseDamage;
	int damageVariance;
	int baseShots;
	int shotsVariance;

	std::string name;

	Character* target;
	float time;
	int currentShot;
	int shotsThisAttack;

	void ExecuteShot(CombatStage* combatStage, Character* execute);
};