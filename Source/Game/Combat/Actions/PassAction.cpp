#include "PassAction.h"

PassAction::PassAction()
{
	this->icon = nullptr;

	this->cost = 0;
	this->requiresTarget = false;
	this->instant = true;
	this->immediatelyEndsTurn = true;
}

std::string PassAction::GetDisplayName() const
{
	return "Pass";
}

void PassAction::SetFromJson(const rapidjson::Value& data)
{
	if (data.HasMember("icon"))
	{
		icon = new Texture(data["icon"].GetString());
	}
}
