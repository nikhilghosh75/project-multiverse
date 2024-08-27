#pragma once

class CombatStage;
class Character;

class Action
{
protected:
	int cost;
	bool requiresTarget = false;

public:
	virtual void Execute(CombatStage* stage, Character* executor);

	virtual void ExecuteOnTarget(CombatStage* stage, Character* executor, Character* target);

	int GetCost() const;
	bool RequiresTarget() const;
};