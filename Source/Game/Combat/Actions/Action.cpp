#include "Action.h"
#include "Combat/CombatStage.h"
#include "Combat/Characters/Character.h"
#include "GuardAction.h"
#include "GunAttack.h"
#include "MeleeAttack.h"
#include "PassAction.h"

Action* Action::CreateFromJson(const rapidjson::Document& data)
{
	Action* action = nullptr;

	if ((std::string)data["type"].GetString() == "melee")
	{
		action = new MeleeAttack();
	}
	else if ((std::string)data["type"].GetString() == "gun")
	{
		action = new GunAttack();
	}
	else if ((std::string)data["type"].GetString() == "guard")
	{
		action = new GuardAction();
	}
	else if ((std::string)data["type"].GetString() == "pass")
	{
		action = new PassAction();
	}

	if (action)
	{
		action->SetFromJson(data);
	}

	return action;
}

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

bool Action::EndsTurn() const
{
	return immediatelyEndsTurn;
}
