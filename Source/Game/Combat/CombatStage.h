#pragma once
#include "Core/Stage.h"
#include "Characters/CompanionCharacter.h"
#include "Characters/PlayerCharacter.h"
#include "Characters/EnemyCharacter.h"
#include "EncounterInfo.h"
#include <vector>

class CombatStage : public Stage
{
public:
	CombatStage(EncounterInfo& info);

	void Update();

	void Render();

	void AdvanceTurn();

	PlayerCharacter* GetPlayerCharacter() { return playerCharacter; }
	std::vector<glm::vec2>& GetEnemyPositions();
	std::vector<EnemyCharacter*>& GetEnemyCharacters();

	Character* GetCurrentTurnCharacter() { return currentTurnCharacter; }

private:
	void ProcessBattleOver();

	void OnBattleLost();

	void OnBattleWon();

	PlayerCharacter* playerCharacter;

	Character* currentTurnCharacter;

	std::vector<EnemyCharacter*> enemies;
	std::vector<CompanionCharacter*> companions;
};