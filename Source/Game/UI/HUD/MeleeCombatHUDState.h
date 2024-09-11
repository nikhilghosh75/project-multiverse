#pragma once
#include "CombatHUD.h"

class MeleeAttack;

class MeleeCombatHUDState : public CombatHUDState
{
public:
	MeleeCombatHUDState();

	void Render(CombatStage* stage);

	void OnTargetSelected(CombatStage* stage, Character* character);
	void OnTurnAdvanced(CombatStage* stage);

	void OnActionEnded(CombatStage* stage, Character* character, Action* action);

private:
	MeleeAttack* selectedAttack;
};