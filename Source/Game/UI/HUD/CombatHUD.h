#pragma once

class CombatStage;
class Action;

class CombatHUDState
{
public:
	CombatHUDState();

	void StartExecuteAction(Action* action, CombatStage* combatStage);

	virtual void Render(CombatStage* stage) = 0;
};

// TODO: Memory cleanup of CombatHUD
class CombatHUD
{
public:
	static void Initialize();

	static void Render(CombatStage* stage);

	static void SetCurrentState(CombatHUDState* newState);

private:
	static inline CombatHUDState* currentState;
};