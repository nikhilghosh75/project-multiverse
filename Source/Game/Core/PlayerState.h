#pragma once

class PlayerCharacter;

class CompanionState
{

};

class PlayerState
{
public:
	void SetFromPlayerCharacter(PlayerCharacter* playerCharacter);

	int health;
	int maxHealth;

	int apPerTurn;
};