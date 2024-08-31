#include "Character.h"
#include "Combat/CombatStage.h"

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
