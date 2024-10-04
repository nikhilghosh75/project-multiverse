#pragma once
#include "Action.h"

class GuardAction : public Action
{
public:
	GuardAction();

	GuardAction(std::string name, int defenseIncrease, int cost);

	GuardAction(std::string name, int defenseIncrease, int cost, std::string iconPath);

	void StartExecute(CombatStage* stage, Character* executor);

	std::string GetDisplayName() const;

	void SetFromJson(const rapidjson::Value& data);

private:
	std::string name;

	int defenseIncrease;
};