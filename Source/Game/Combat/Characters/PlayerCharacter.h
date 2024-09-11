#pragma once
#include "Character.h"
#include "Core/PlayerState.h"

class PlayerCharacter : public Character
{
public:
	PlayerCharacter();
	PlayerCharacter(PlayerState* playerState);

	void Render();

	void EndAction(CombatStage* stage);
private:
	PlayerState* playerState;

	void AddTempAbilities();
};