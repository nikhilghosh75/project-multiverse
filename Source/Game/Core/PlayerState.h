#pragma once
#include <vector>

class PlayerCharacter;
class CompanionCharacter;

class PlayerState
{
public:
	void SetFromPlayerCharacter(PlayerCharacter* playerCharacter);

	std::vector<CompanionCharacter*> companions;

	int health;
	int maxHealth;

	int apPerTurn;
};