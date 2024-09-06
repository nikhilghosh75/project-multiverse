#include "Character.h"
#include "Combat/CombatStage.h"

Character::Character()
{
	maxHealth = 0;
	health = 0;
	texture = nullptr;
	actionPoints = 0;
	defense = 0;
}

Character::~Character()
{

}

int Character::Damage(int damage)
{
	int actualDamage = damage > defense ? damage - defense : 0;
	health -= actualDamage;

	return actualDamage;
}

void Character::DeductActionPoints(int actionCost)
{
	actionPoints -= actionCost;

	if (actionPoints < 0)
		actionPoints = 0;
}

void Character::OnTurnUpdate(CombatStage* stage)
{
	if (currentAction != nullptr)
	{
		currentAction->UpdateExecute(stage, this);
	}
}

void Character::StartAction(Action* action)
{
	currentAction = action;
}

void Character::EndAction(CombatStage* stage)
{
	currentAction = nullptr;

	if (actionPoints <= 0)
	{
		stage->AdvanceTurn();
	}
}

int Character::GetHealth() const
{
	return health;
}

int Character::GetMaxHealth() const
{
	return maxHealth;
}

int Character::GetActionPoints() const
{
	return actionPoints;
}

int Character::GetDefense() const
{
	return defense;
}

void Character::IncreaseDefense(int defenseIncrease)
{
	defense += defenseIncrease;
}

bool Character::IsDead() const
{
	return health >= 0;
}
