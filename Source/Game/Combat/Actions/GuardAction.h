#pragma once
#include "Action.h"

class GuardAction : public Action
{
public:
	GuardAction(std::string name, int defenseIncrease, int cost);

	void StartExecute(CombatStage* stage, Character* executor);

	std::string GetDisplayName() const;

private:
	std::string name;

	int defenseIncrease;
};