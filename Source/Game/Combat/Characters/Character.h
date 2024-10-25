#pragma once
#include <string>
#include <memory>
#include "Combat/Actions/Action.h"
#include "glm/glm.hpp"
#include "Texture.h"
#include "rapidjson/document.h"

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

	virtual ~Character();

	CharacterType type;
	std::string name;

	glm::vec2 baseScreenPosition;
	glm::vec2 screenOffset;

	virtual int Damage(int damage); // Returns the actual amount of damage done
	virtual void DeductActionPoints(int actionPoints);

	virtual void OnTurnStart(CombatStage* stage);

	virtual void OnTurnUpdate(CombatStage* stage);

	virtual void OnDeath() { }

	virtual void StartAction(std::shared_ptr<Action> action);
	virtual void EndAction(CombatStage* stage);

	int GetHealth() const;
	int GetMaxHealth() const;
	int GetActionPoints() const;
	int GetDefense() const;

	void IncreaseDefense(int defenseIncrease);

	bool IsDead() const;

	std::vector<std::shared_ptr<Action>> actions;

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
	virtual void SetFromJsonData(const rapidjson::Document& data);

	int maxHealth;
	int health;
	int actionPoints;
	
	int actionPointsPerTurn;

	int defense;

	Texture* texture;
	std::shared_ptr<Action> currentAction;
};