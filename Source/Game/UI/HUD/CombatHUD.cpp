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

void CombatHUD::SetCurrentState(CombatHUDState* newState)
{
	currentState = newState;
}

CombatHUDState::CombatHUDState()
{
}

void CombatHUDState::StartExecuteAction(Action* action, CombatStage* combatStage)
{
	if (action->RequiresTarget())
	{
		std::cout << "Start Target Selection Process" << std::endl;
	}
	else
	{
		action->Execute(combatStage);;
	}
}
