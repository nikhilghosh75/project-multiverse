#pragma once
#include "CombatHUD.h"

class GuardCombatHUDState : public CombatHUDState
{
public:
	GuardCombatHUDState();

	void Render(CombatStage* stage);

	void OnTurnAdvanced(CombatStage* stage);
};