#pragma once
#include "CombatHUD.h"

class MainCombatHUDState : public CombatHUDState
{
public:
	MainCombatHUDState();

	void Render(CombatStage* stage);

private:
	void OnTurnAdvanced(CombatStage* stage);

	void OnTargetSelected(CombatStage* stage, Character* character);

	static inline Texture* noIconTexture;
};