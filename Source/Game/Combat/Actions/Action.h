#pragma once
#include <string>

class CombatStage;
class Character;

class Action
{
protected:
	int cost;
	bool requiresTarget = false;
	bool instant = false;

public:
	virtual void StartExecute(CombatStage* stage, Character* executor);

	virtual void StartExecuteOnTarget(CombatStage* stage, Character* executor, Character* target);

	virtual void UpdateExecute(CombatStage* stage, Character* executor);

	virtual std::string GetDisplayName() const = 0;

	int GetCost() const;
	bool RequiresTarget() const;
};