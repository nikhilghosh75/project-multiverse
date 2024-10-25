#pragma once
#include <memory>
#include <string>
#include "Texture.h"
#include "rapidjson/document.h"

class ActionVisual;
class CombatStage;
class Character;

class Action
{
protected:
	int cost;
	bool requiresTarget = false;
	bool instant = false;
	bool immediatelyEndsTurn = false;

	Texture* icon;
	ActionVisual* visual;
public:
	static std::shared_ptr<Action> CreateFromJson(const rapidjson::Value& data);

	virtual void StartExecute(CombatStage* stage, Character* executor);

	virtual void StartExecuteOnTarget(CombatStage* stage, Character* executor, Character* target);

	virtual void UpdateExecute(CombatStage* stage, Character* executor);

	virtual std::string GetDisplayName() const = 0;

	virtual void SetFromJson(const rapidjson::Value& data) = 0;

	Texture* GetTexture() const;
	int GetCost() const;
	bool RequiresTarget() const;
	bool EndsTurn() const;
	bool Instant() const;

private:
	static ActionVisual* CreateVisualFromJson(const rapidjson::Value& data);

	Character* currentTarget;
};

class ActionVisual
{
public:
	virtual void Start(Action* action, Character* executor, Character* target);

	virtual void Update(Action* action, Character* executor, Character* target);

	virtual void End(Action* action, Character* executor, Character* target);

	virtual float GetVisualTime() const { return 0.0f; }
};