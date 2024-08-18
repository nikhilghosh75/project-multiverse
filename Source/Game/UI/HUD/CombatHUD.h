#pragma once

class CombatStage;

class CombatHUDState
{
public:
	CombatHUDState();

	virtual void Render(CombatStage* stage) = 0;
};

class CombatHUD
{
public:
	static void Initialize();

	static void Render(CombatStage* stage);

private:
	static inline CombatHUDState* currentState;
};