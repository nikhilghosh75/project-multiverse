#pragma once

class CombatStage;
class Character;

class Action
{
protected:
	int cost;
	bool requiresTarget = false;

public:
	virtual void Execute(CombatStage* stage);

	virtual void ExecuteOnTarget(CombatStage* stage, Character* character);

	int GetCost() const;
	bool RequiresTarget() const;
};