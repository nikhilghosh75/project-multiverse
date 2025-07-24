#include "CRC.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <set>

/*
Usage: inFilepath outFilepath
*/

const size_t BUFFER_SIZE = 1048576;
const size_t LINE_SIZE = 1024;

const uint8_t CURRENT_VERSION = 1;

const size_t HEADER_SIZE = 40;
const int RESERVED_WORDS = 128; // Codes 0-RESERVED_WORDS are reserved for specific macros

struct LocalizationHeader
{
	uint32_t numberOfWords;
	uint32_t numberOfEntries;
	uint32_t wordSectionLength;
	uint32_t entriesSectionLength;
	uint8_t wordsLengthPadding;
	uint8_t versionNumber;

	char padding[HEADER_SIZE - 18];
};

uint16_t GetNumWordsInLine(char* str)
{
	uint16_t numWords = 0;
	int currentIndex = 0;
	while (str[currentIndex] != '\0')
	{
		if (str[currentIndex] != ' ')
		{
			numWords++;
		}
		currentIndex++;
	}

	return numWords;
}

int main(int argc, char** argv)
{
	if (argc < 3)
	{
		std::cout << argc << std::endl;
		std::cout << "Usage: ./LocalizationConverter inFilepath outFilepath" << std::endl;
		return 1;
	}

	// Parse Arguments
	char* inFilepath = argv[1];
	char* outFilepath = argv[2];
	
	// Initialize File
	std::ifstream inFile(inFilepath);
	std::ofstream outFile(outFilepath, std::ios::app | std::ios::binary);

	char line[LINE_SIZE];
	std::memset(line, 0, LINE_SIZE);
  
	std::map<std::string, uint16_t> words;
	uint32_t numWords = 0; 
	uint32_t currentWord = RESERVED_WORDS;

	std::set<uint32_t> setOfCodes;
	uint32_t numLines = 0;

	// To output to file
	char* wordsSection = (char*)malloc(BUFFER_SIZE);
	size_t currentWordPosition = 0;

	char* entriesSection = (char*)malloc(BUFFER_SIZE);
	size_t currentEntryPosition = 0;

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
		uint16_t numWordsInLine = GetNumWordsInLine(valueStr);

		if (setOfCodes.find(crc) != setOfCodes.end())
		{
			// TODO: Hook into the error system
			std::cout << "Conflict " << keyStr << std::endl;
		}

		setOfCodes.insert(crc);
		memcpy(entriesSection + currentEntryPosition, &crc, sizeof(crc));
		memcpy(entriesSection + currentEntryPosition + 4, &numWordsInLine, sizeof(numWordsInLine));
		currentEntryPosition += 6;
		numLines++;

		// Insert words in the string
		int i = 0;
		int lastWordIndex = 0;
		while (valueStr[i] != '\0')
		{
			if (valueStr[i] == ' ')
			{
				// This is a word. Find it and store it
				std::string word = std::string(&valueStr[lastWordIndex], i - lastWordIndex);
				auto foundWord = words.find(word);
				if (foundWord == words.end())
				{
					uint8_t wordSize = (uint8_t)word.size();
					wordsSection[currentWordPosition] = *((char*)&wordSize);
					memcpy(wordsSection + currentWordPosition + 1, word.c_str(), word.size());
					currentWordPosition += word.size() + 1;

					words.insert({ word, currentWord });
					foundWord = words.find(word);
					currentWord++;
				}

				// Add to the entries list
				uint16_t wordIndex = (*foundWord).second;
				memcpy(entriesSection + currentEntryPosition, &wordIndex, sizeof(wordIndex));
				currentEntryPosition += 2;

				lastWordIndex = i + 1;
				numWords++;
			}
			i++;
		}

		std::memset(line, 0, LINE_SIZE);
	}

	// We add padding between the words and entries to ensure the ensures always start on a multiple of 4
	uint8_t wordsPadding = currentWordPosition % 4 == 0 ? 0 : 4 - (currentWordPosition % 4);

	LocalizationHeader header;
	header.numberOfWords = numWords;
	header.numberOfEntries = numLines;
	header.entriesSectionLength = currentEntryPosition;
	header.wordsLengthPadding = wordsPadding;
	header.wordSectionLength = currentWordPosition;
	header.versionNumber = CURRENT_VERSION;

	outFile.write("LOCALIZATION", 12);
	outFile.write((char*)&header, sizeof(LocalizationHeader));
	outFile.write(wordsSection, currentWordPosition);
	outFile.write("    ", wordsPadding);
	outFile.write(entriesSection, currentEntryPosition);

	free(wordsSection);
	free(entriesSection);
}