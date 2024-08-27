#pragma once
#include "CombatHUD.h"

class EnemyCharacter;

class EnemyTurnHUDState : public CombatHUDState
{
public:
	EnemyTurnHUDState();

	void Render(CombatStage* stage);
};