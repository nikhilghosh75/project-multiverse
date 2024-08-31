#include "Action.h"
#include "Combat/CombatStage.h"
#include "Combat/Characters/Character.h"

void Action::StartExecute(CombatStage* stage, Character* executor)
{
}

void Action::StartExecuteOnTarget(CombatStage* stage, Character* executor, Character* target)
{
}

void Action::UpdateExecute(CombatStage* stage, Character* executor)
{
}

int Action::GetCost() const
{
	return cost;
}

bool Action::RequiresTarget() const
{
	return requiresTarget;
}
