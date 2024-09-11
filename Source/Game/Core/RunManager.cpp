#include "RunManager.h"

const int startingHealth = 90;
const int startingAPPerTurn = 3;

void RunManager::StartRun()
{
	isInRun = true;

	playerState = new PlayerState();
	playerState->health = startingHealth;
	playerState->maxHealth = startingHealth;
	playerState->apPerTurn = startingAPPerTurn;
}
