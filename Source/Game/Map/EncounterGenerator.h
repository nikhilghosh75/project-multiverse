#pragma once
#include "Combat/EncounterInfo.h"
#include "Combat/Characters/CompanionCharacter.h"
#include "Combat/Characters/EnemyCharacter.h"
#include <vector>

class EncounterGenerator
{
public:
	static void Initialize();

	static EncounterInfo Generate(int encounterNumber);

	static CompanionCharacter* GetNewCompanion();

private:
	static inline std::vector<EnemyCharacter*> enemyCharacters;
	static inline std::vector<CompanionCharacter*> companionCharacters;

	static std::vector<EnemyCharacter*> GetCharactersBelowPowerRating(int powerRating);
};