#include "RunManager.h"
#include "Combat/CombatStage.h"

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

void RunManager::OnBattleOver(CombatStage* stage)
{
	playerState->SetFromPlayerCharacter(stage->GetPlayerCharacter());
}
