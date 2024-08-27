#include "Character.h"

Character::Character()
{
	health = 0;
	texture = nullptr;
	actionPoints = 0;
}

Character::~Character()
{

}

void Character::Damage(int damage)
{
	health -= damage;
}

void Character::DeductActionPoints(int actionCost)
{
	actionPoints -= actionCost;

	if (actionPoints < 0)
		actionPoints = 0;
}

int Character::GetHealth() const
{
	return health;
}

int Character::GetActionPoints() const
{
	return actionPoints;
}

bool Character::IsDead() const
{
	return health >= 0;
}

void Character::Setup(int startingHealth)
{
	health = startingHealth;
}
