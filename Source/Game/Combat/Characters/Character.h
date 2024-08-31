#pragma once
#include <string>
#include "Combat/Actions/Action.h"
#include "glm/glm.hpp"
#include "Texture.h"

class CombatStage;

enum class CharacterType
{
	Player,
	Companion,
	Enemy
};

class Character
{
public:
	Character();

	~Character();

	CharacterType type;
	std::string name;

	glm::vec2 screenPosition;

	virtual void Damage(int damage);
	virtual void DeductActionPoints(int actionPoints);

	virtual void OnTurnStart(CombatStage* stage) {}

	virtual void OnTurnUpdate(CombatStage* stage);

	void StartAction(Action* action);
	void EndAction(CombatStage* stage);

	int GetHealth() const;
	int GetActionPoints() const;

	bool IsDead() const;

	std::vector<Action*> actions;

private:
	int health;
	int actionPoints;

protected:
	void Setup(int startingHealth);

	Texture* texture;
	Action* currentAction;
};