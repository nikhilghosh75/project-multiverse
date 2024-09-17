#pragma once
#include <string>
#include "rapidjson/document.h"

class CombatStage;
class Character;

class Action
{
protected:
	int cost;
	bool requiresTarget = false;
	bool instant = false;
	bool immediatelyEndsTurn = false;

public:
	static Action* CreateFromJson(const rapidjson::Value& data);

	virtual void StartExecute(CombatStage* stage, Character* executor);

	virtual void StartExecuteOnTarget(CombatStage* stage, Character* executor, Character* target);

	virtual void UpdateExecute(CombatStage* stage, Character* executor);

	virtual std::string GetDisplayName() const = 0;

	virtual void SetFromJson(const rapidjson::Value& data) = 0;

	int GetCost() const;
	bool RequiresTarget() const;
	bool EndsTurn() const;
};