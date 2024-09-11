#pragma once
#include "Texture.h"
#include "Combat/Actions/MeleeAttack.h"
#include <vector>

class EnemyInfo
{
public:
	Texture* texture;
	const char* enemyName;
	int startingHealth;

	MeleeAttack* enemyAttack;
};

class EncounterInfo
{
public:
	std::vector<EnemyInfo> enemies;
};