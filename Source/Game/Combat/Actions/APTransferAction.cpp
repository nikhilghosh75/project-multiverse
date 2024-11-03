#include "APTransferAction.h"
#include "Combat/CombatStage.h"

APTransferAction::APTransferAction()
	: transferAmount(0)
{
	this->cost = 0;

	this->instant = true;
	this->requiresTarget = false;
	this->immediatelyEndsTurn = true;
}

APTransferAction::APTransferAction(int transferAmount)
	: transferAmount(transferAmount)
{
	this->cost = 0;

	this->instant = true;
	this->requiresTarget = false;
	this->immediatelyEndsTurn = true;
}

void APTransferAction::StartExecute(CombatStage* stage, Character* executor)
{
	stage->GetPlayerCharacter()->IncreaseActionPoints(transferAmount);
	executor->DeductActionPoints(transferAmount);
}

std::string APTransferAction::GetDisplayName() const
{
	return "Transfer " + std::to_string(transferAmount) + " AP";
}

void APTransferAction::SetFromJson(const rapidjson::Value& data)
{
	transferAmount = data["transfer_amount"].GetInt();

	if (data.HasMember("icon"))
	{
		icon = new Texture(data["icon"].GetString());
	}
}
