#include "RunManager.h"
#include "Combat/CombatStage.h"
#include "Map/EncounterGenerator.h"

const int startingHealth = 90;
const int startingAPPerTurn = 3;

void RunManager::StartRun()
{
	isInRun = true;

	playerState = PlayerState();
	playerState.health = startingHealth;
	playerState.maxHealth = startingHealth;
	playerState.apPerTurn = startingAPPerTurn;

	EncounterGenerator::Initialize();

	// TODO: Move to a physical map
	EncounterInfo info = EncounterGenerator::Generate(0);
	StageManager::AddStage(new CombatStage(info));
}

void RunManager::OnBattleStart(CombatStage* stage)
{
	currentEncounter++;
}

void RunManager::OnBattleOver(CombatStage* stage)
{
	playerState.SetFromPlayerCharacter(stage->GetPlayerCharacter());
}
