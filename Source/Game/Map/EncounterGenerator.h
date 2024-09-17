#pragma once
#include "Combat/EncounterInfo.h"
#include "Combat/Characters/EnemyCharacter.h"
#include <vector>

class EncounterGenerator
{
public:
	static void Initialize();

	static EncounterInfo Generate(int encounterNumber);

private:
	static inline std::vector<EnemyCharacter*> enemyCharacters;

	static std::vector<EnemyCharacter*> GetCharactersBelowPowerRating(int powerRating);
};