#include "CombatHUD.h"
#include "Combat/CombatStage.h"
#include "FontRenderer.h"
#include "UI/HUD/MainCombatHUDState.h"

#include "Input.h"
#include "Window.h"
#include <iostream>

void CombatHUD::Initialize()
{
	currentState = new MainCombatHUDState();
}

void CombatHUD::Render(CombatStage* stage)
{
	currentState->Render(stage);
}

CombatHUDState::CombatHUDState()
{
}
