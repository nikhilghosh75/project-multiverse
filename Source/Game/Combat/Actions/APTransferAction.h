#pragma once
#include "Combat/Actions/Action.h"

class APTransferAction : public Action
{
public:
	APTransferAction();

	APTransferAction(int transferAmount);

	void StartExecute(CombatStage* stage, Character* executor);

	std::string GetDisplayName() const;

	void SetFromJson(const rapidjson::Value& data);

private:
	int transferAmount;
};