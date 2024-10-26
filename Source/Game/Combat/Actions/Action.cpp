#include "Action.h"
#include "Combat/CombatStage.h"
#include "Combat/Characters/Character.h"
#include "CrashActionVisual.h"
#include "GuardAction.h"
#include "GunAttack.h"
#include "MeleeAttack.h"
#include "PassAction.h"
#include "ProjectileActionVisual.h"

Action::~Action()
{
	if (visual != nullptr)
	{
		delete visual;
	}

	if (icon != nullptr)
	{
		delete icon;
	}
}

std::shared_ptr<Action> Action::CreateFromJson(const rapidjson::Value& data)
{
	std::shared_ptr<Action> action;

	if ((std::string)data["type"].GetString() == "melee")
	{
		action = std::make_shared<MeleeAttack>();
	}
	else if ((std::string)data["type"].GetString() == "gun")
	{
		action = std::make_shared<GunAttack>();
	}
	else if ((std::string)data["type"].GetString() == "guard")
	{
		action = std::make_shared<GuardAction>();
	}
	else if ((std::string)data["type"].GetString() == "pass")
	{
		action = std::make_shared<PassAction>();
	}

	if (action)
	{
		action->SetFromJson(data);
		action->visual = CreateVisualFromJson(data);
	}

	return action;
}

void Action::StartExecute(CombatStage* stage, Character* executor)
{
	if (visual != nullptr)
	{
		visual->Start(this, executor, nullptr);
	}
}

void Action::StartExecuteOnTarget(CombatStage* stage, Character* executor, Character* target)
{
	currentTarget = target;

	if (visual != nullptr)
	{
		visual->Start(this, executor, target);
	}
}

void Action::UpdateExecute(CombatStage* stage, Character* executor)
{
	if (visual != nullptr)
	{
		visual->Update(this, executor, currentTarget);
	}
}

void Action::SetActionVisual(ActionVisual* _actionVisual)
{
	visual = _actionVisual;
}

Texture* Action::GetTexture() const
{
	return icon;
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

bool Action::Instant() const
{
	return instant;
}

ActionVisual* Action::CreateVisualFromJson(const rapidjson::Value& data)
{
	ActionVisual* actionVisual = nullptr;

	if (!data.HasMember("visual_type"))
	{
		return nullptr;
	}

	if (strcmp(data["visual_type"].GetString(), "crash") == 0)
	{
		actionVisual = new CrashActionVisual(data);
	}

	if (strcmp(data["visual_type"].GetString(), "projectile") == 0)
	{
		actionVisual = new ProjectileActionVisual(data);
	}

	return actionVisual;
}

void ActionVisual::Start(Action* action, Character* executor, Character* target)
{
}

void ActionVisual::Update(Action* action, Character* executor, Character* target)
{

}

void ActionVisual::End(Action* action, Character* executor, Character* target)
{
}
