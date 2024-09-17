#include "PlayerState.h"
#include "Combat/Characters/PlayerCharacter.h"

void PlayerState::SetFromPlayerCharacter(PlayerCharacter* playerCharacter)
{
	health = playerCharacter->GetHealth();
}
