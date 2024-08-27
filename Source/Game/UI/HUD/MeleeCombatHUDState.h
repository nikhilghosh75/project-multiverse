#pragma once
#include "CombatHUD.h"

class MeleeAttack;

class MeleeCombatHUDState : public CombatHUDState
{
public:
	MeleeCombatHUDState();

	void Render(CombatStage* stage);

	void OnTargetSelected(CombatStage* stage, Character* character);

private:
	MeleeAttack* selectedAttack;
};