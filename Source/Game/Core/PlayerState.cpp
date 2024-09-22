#include "PlayerState.h"
#include "Combat/Characters/PlayerCharacter.h"

void PlayerState::SetFromPlayerCharacter(PlayerCharacter* playerCharacter)
{
	health = playerCharacter->GetHealth();
}

void PlayerState::RemoveCompanion(CompanionCharacter* companionCharacter)
{
	auto it = std::find(companions.begin(), companions.end(), companionCharacter);
	if (it != companions.end())
	{
		companions.erase(it);
	}
}
