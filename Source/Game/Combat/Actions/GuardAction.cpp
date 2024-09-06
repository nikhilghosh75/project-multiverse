#include "GuardAction.h"
#include "Combat/Characters/Character.h"

GuardAction::GuardAction(std::string name, int defenseIncrease, int cost)
	: name(name), defenseIncrease(defenseIncrease)
{
	this->cost = cost;

	this->instant = true;
	this->requiresTarget = false;
}

void GuardAction::StartExecute(CombatStage* stage, Character* executor)
{
	executor->IncreaseDefense(defenseIncrease);
}

std::string GuardAction::GetDisplayName() const
{
	return name + "(+" + std::to_string(defenseIncrease) + " DEF, " + std::to_string(cost) + " AP)";
}
