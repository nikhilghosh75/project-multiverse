#pragma once
#include "PlayerState.h"

class CombatStage;

class RunManager
{
public:
	static bool IsInRun() { return isInRun; }

	static void StartRun();

	static void OnBattleOver(CombatStage* stage);

	static PlayerState* GetPlayerState() { return playerState; }
private:
	static inline bool isInRun = false;

	static inline PlayerState* playerState;
};