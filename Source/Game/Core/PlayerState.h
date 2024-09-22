#pragma once
#include <vector>

class PlayerCharacter;
class CompanionCharacter;

class PlayerState
{
public:
	void SetFromPlayerCharacter(PlayerCharacter* playerCharacter);

	void RemoveCompanion(CompanionCharacter* companionCharacter);

	std::vector<CompanionCharacter*> companions;

	int health;
	int maxHealth;

	int apPerTurn;
};