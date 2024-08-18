#include "Action.h"
#include "Combat/CombatStage.h"
#include "Combat/Characters/Character.h"

void Action::Execute(CombatStage* stage)
{
}

void Action::ExecuteOnTarget(CombatStage* stage, Character* character)
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
