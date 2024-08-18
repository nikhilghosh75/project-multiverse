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

int Character::GetHealth() const
{
	return health;
}

int Character::GetActionPoints() const
{
	return actionPoints;
}

void Character::Setup(int startingHealth)
{
	health = startingHealth;
}
