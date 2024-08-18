#pragma once
#include "CombatHUD.h"

class MainCombatHUDState : public CombatHUDState
{
public:
	MainCombatHUDState();

	void Render(CombatStage* stage);

private:
	void OnMeleeButtonClicked();

	void OnGunButtonClicked();

	void OnGuardButtonClicked();

	void OnPassButtonClicked();
};