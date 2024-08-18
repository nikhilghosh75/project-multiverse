#pragma once
#include <string>
#include "Combat/Actions/Action.h"
#include "glm/glm.hpp"
#include "Texture.h"

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

	int GetHealth() const;
	int GetActionPoints() const;

	std::vector<Action*> actions;

private:
	int health;
	int actionPoints;

protected:
	void Setup(int startingHealth);

	Texture* texture;
};