#include "LocalizationSystem.h"

#include "CRC.h"

#include <fstream>

const size_t LINE_SIZE = 1024;

LocalizationSystem* LocalizationSystem::Get()
{
	if (instance == nullptr)
	{
		instance = new LocalizationSystem();
	}

	return instance;
}

void LocalizationSystem::LoadLanguage(std::string filepath)
{
	// TODO: Add Binary Files
	LoadLanguageFromTextFile(filepath);
}

const char* LocalizationSystem::LocalizeString(const std::string& key)
{
	uint32_t crc = CRC::Calculate(key.c_str(), key.size());

	auto foundIt = loadedLanguages[currentLanguageIndex].langaugeMap.find(crc);

	if (foundIt != loadedLanguages[currentLanguageIndex].langaugeMap.end())
	{
		return foundIt->second;
	}

	return nullptr;
}

void LocalizationSystem::LoadLanguageFromTextFile(std::string filepath)
{
	size_t bytesNeeded = 0;
	int numKeys = 0;

	LocalizedLanguage language;

	std::ifstream inFile(filepath);

	char line[LINE_SIZE];
	std::memset(line, 0, LINE_SIZE);

	// First Pass: Get the total amount of memory needed to allocate for the strings
	size_t languageSize = 0;

	while (inFile.getline(line, LINE_SIZE))
	{
		int length = strlen(line);
		int equalsIndex = 0;

		for (int i = 0; i < length; i++)
		{
			if (line[i] == '=')
			{
				equalsIndex = i;
				break;
			}
		}

		// If line doesn't have an equals sign, skip it
		if (equalsIndex == 0)
		{
			continue;
		}

		char* valueStr = line + equalsIndex + 2;
		int valueLength = strlen(valueStr);

		languageSize += valueLength + 2;

		std::memset(line, 0, LINE_SIZE);
	}

	language.languageLocation = malloc(languageSize);
	
	size_t currentPosition = 0;

	// Reset the ifstream for the second pass
	inFile.clear();
	inFile.seekg(0);

	// Second Pass: Actually copy the strings
	while (inFile.getline(line, LINE_SIZE))
	{
		int length = strlen(line);
		int equalsIndex = 0;

		for (int i = 0; i < length; i++)
		{
			if (line[i] == '=')
			{
				equalsIndex = i;
				break;
			}
		}

		// If line doesn't have an equals sign, skip it
		if (equalsIndex == 0)
		{
			continue;
		}

		line[equalsIndex - 1] = '\0';
		char* keyStr = line;
		char* valueStr = line + equalsIndex + 2;

		uint32_t crc = CRC::Calculate(keyStr, equalsIndex - 1);
		int valueLength = strlen(valueStr);

		char* strLocation = (char*)language.languageLocation + currentPosition;
		memcpy(strLocation, valueStr, valueLength);
		*(strLocation + valueLength) = '\0';
		language.langaugeMap.insert({ crc, strLocation });

		currentPosition += valueLength + 1;
		std::memset(line, 0, LINE_SIZE);
	}

	loadedLanguages.push_back(language);
}
