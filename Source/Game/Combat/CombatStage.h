#pragma once
#include "Core/Stage.h"
#include "Characters/PlayerCharacter.h"
#include "Characters/EnemyCharacter.h"
#include "EncounterInfo.h"

class CombatStage : public Stage
{
public:
	CombatStage(EncounterInfo& info);

	void Update();

	void Render();

	PlayerCharacter* GetPlayerCharacter() { return playerCharacter; }

private:
	PlayerCharacter* playerCharacter;

	std::vector<EnemyCharacter*> enemies;
};