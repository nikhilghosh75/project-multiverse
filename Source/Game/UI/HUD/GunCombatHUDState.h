#pragma once

#include "CombatHUD.h"

class GunAttack;

class GunCombatHUDState : public CombatHUDState
{
public:
	GunCombatHUDState();

	void Render(CombatStage* stage);

	void OnTargetSelected(CombatStage* stage, Character* character);
	void OnTurnAdvanced(CombatStage* stage);

private:
	GunAttack* selectedAttack;
};