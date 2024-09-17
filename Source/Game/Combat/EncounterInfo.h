#pragma once
#include "Texture.h"
#include "Combat/Characters/EnemyCharacter.h"
#include <vector>

class EnemyInfo
{
public:
	EnemyInfo(EnemyCharacter* character);

	EnemyCharacter* character;
};

class EncounterInfo
{
public:
	std::vector<EnemyInfo> enemies;
};