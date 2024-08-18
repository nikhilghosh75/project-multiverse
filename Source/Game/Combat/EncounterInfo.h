#pragma once
#include "Texture.h"
#include <vector>

class EnemyInfo
{
public:
	Texture* texture;
	const char* enemyName;
};

class EncounterInfo
{
public:
	std::vector<EnemyInfo> enemies;
};