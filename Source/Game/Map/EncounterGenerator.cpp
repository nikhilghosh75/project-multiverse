#include "EncounterGenerator.h"
#include "rapidjson/document.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <cstdlib>

// TODO: Make a better system
const std::vector<int> powerRatingsPerEncounter = { 3, 9, 15 };

void EncounterGenerator::Initialize()
{
	std::filesystem::path path("Data/Characters");

	for (auto const& dir_entry : std::filesystem::directory_iterator{ path })
	{
		std::size_t size = std::filesystem::file_size(dir_entry.path());

		std::ifstream file(dir_entry.path().c_str());
		file.seekg(0, std::ios::beg);

		std::vector<char> buffer(size);
		file.read(buffer.data(), size);
		if (size != 0)
		{
			rapidjson::Document d;
			d.Parse(buffer.data());

			if (strcmp(d["type"].GetString(), "enemy") == 0)
			{
				EnemyCharacter* character = new EnemyCharacter(d);
				enemyCharacters.push_back(character);
			}
			else if (strcmp(d["type"].GetString(), "companion") == 0)
			{
				CompanionCharacter* character = new CompanionCharacter(d);
				companionCharacters.push_back(character);
			}
		}
	}
}

EncounterInfo EncounterGenerator::Generate(int encounterNumber)
{
	EncounterInfo info;

	const int MAX_ENEMIES = 2;

	int currentPowerRating = powerRatingsPerEncounter[encounterNumber];

	bool canAddAdditionalEnemies = true;
	while (canAddAdditionalEnemies)
	{
		std::vector<EnemyCharacter*> availableCharacters = GetCharactersBelowPowerRating(currentPowerRating);

		if (availableCharacters.size() > 0)
		{
			int enemyIndex = std::rand() % availableCharacters.size();
			info.enemies.emplace_back(availableCharacters[enemyIndex]);

			currentPowerRating -= availableCharacters[enemyIndex]->powerRating;

			if (currentPowerRating <= 0)
			{
				canAddAdditionalEnemies = false;
			}
			if (availableCharacters.size() >= MAX_ENEMIES)
			{
				canAddAdditionalEnemies = false;
			}
		}
		else
		{
			canAddAdditionalEnemies = false;
		}
	}

	return info;
}

CompanionCharacter* EncounterGenerator::GetNewCompanion()
{
	if (companionCharacters.size() == 0)
	{
		return nullptr;
	}

	CompanionCharacter* baseCompanion = companionCharacters[std::rand() % companionCharacters.size()];
	
	return new CompanionCharacter(baseCompanion);
}

std::vector<EnemyCharacter*> EncounterGenerator::GetCharactersBelowPowerRating(int powerRating)
{
	std::vector<EnemyCharacter*> characters;

	for (int i = 0; i < enemyCharacters.size(); i++)
	{
		if (enemyCharacters[i]->powerRating <= powerRating)
		{
			characters.push_back(enemyCharacters[i]);
		}
	}

	return characters;
}
