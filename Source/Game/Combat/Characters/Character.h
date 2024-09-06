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

	virtual int Damage(int damage); // Returns the actual amount of damage done
	virtual void DeductActionPoints(int actionPoints);

	virtual void OnTurnStart(CombatStage* stage) {}

	virtual void OnTurnUpdate(CombatStage* stage);

	void StartAction(Action* action);
	void EndAction(CombatStage* stage);

	int GetHealth() const;
	int GetMaxHealth() const;
	int GetActionPoints() const;
	int GetDefense() const;

	void IncreaseDefense(int defenseIncrease);

	bool IsDead() const;

	std::vector<Action*> actions;

	template<typename T> T* FindFirstActionOfType()
	{
		for (int i = 0; i < actions.size(); i++)
		{
			T* t = dynamic_cast<T*>(actions[i]);

			if (t)
			{
				return t;
			}
		}

		return nullptr;
	}

protected:
	int maxHealth;
	int health;
	int actionPoints;

	int defense;

	Texture* texture;
	Action* currentAction;
};