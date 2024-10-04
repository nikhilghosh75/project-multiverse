#include "GuardAction.h"
#include "Combat/Characters/Character.h"

GuardAction::GuardAction()
{
	this->instant = true;
	this->requiresTarget = false;
	this->immediatelyEndsTurn = true;
}

GuardAction::GuardAction(std::string name, int defenseIncrease, int cost)
	: name(name), defenseIncrease(defenseIncrease)
{
	this->cost = cost;

	this->instant = true;
	this->requiresTarget = false;
	this->immediatelyEndsTurn = true;
}

GuardAction::GuardAction(std::string name, int defenseIncrease, int cost, std::string iconPath)
	: name(name), defenseIncrease(defenseIncrease)
{
	this->cost = cost;
	this->icon = new Texture(iconPath);

	this->instant = true;
	this->requiresTarget = false;
	this->immediatelyEndsTurn = true;
}

void GuardAction::StartExecute(CombatStage* stage, Character* executor)
{
	executor->IncreaseDefense(defenseIncrease);
	executor->DeductActionPoints(cost);
}

std::string GuardAction::GetDisplayName() const
{
	return name + "(+" + std::to_string(defenseIncrease) + " DEF)";
}

void GuardAction::SetFromJson(const rapidjson::Value& data)
{
	name = data["name"].GetString();
	defenseIncrease = data["defense_increase"].GetInt();
	cost = data["cost"].GetInt();

	if (data.HasMember("icon"))
	{
		icon = new Texture(data["icon"].GetString());
	}
}
