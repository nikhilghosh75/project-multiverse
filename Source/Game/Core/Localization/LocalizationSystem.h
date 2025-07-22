#pragma once

#include <unordered_map>
#include <string>

struct LocalizedLanguage
{
	// Converts CRC keys to their localized string
	std::unordered_map<uint32_t, char*> langaugeMap;
	char* keyNotFoundStr;

	// All of the memory for a language is allocated all at once
	void* languageLocation;
};

class LocalizationSystem
{
public:
	static LocalizationSystem* Get();

	void LoadLanguage(std::string filepath);

	const char* LocalizeString(const std::string& key);
private:
	static inline LocalizationSystem* instance;

	std::vector<LocalizedLanguage> loadedLanguages;
	int currentLanguageIndex = 0;

	void LoadLanguageFromTextFile(std::string filepath);

	void LoadLanguageFromBinaryFile(std::string filepath);
};