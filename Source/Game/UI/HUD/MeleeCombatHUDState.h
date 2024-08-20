#pragma once
#include "CombatHUD.h"

class MeleeCombatHUDState : public CombatHUDState
{
public:
	MeleeCombatHUDState();

	void Render(CombatStage* stage);
};