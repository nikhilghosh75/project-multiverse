#include "CRC.h"
#include <fstream>
#include <iostream>
#include <map>
#include <set>

const int RESERVED_WORDS = 128;

int main(char** argv, int argc)
{
	std::ifstream tempStream("C:/Users/debgh/source/repos/College Basketball Simuator/Data/English.txt");
	char line[1024];
	std::memset(line, 0, 1024);
  
	std::map<std::string, uint16_t> words;
	int numWords = 0;
	int currentWord = RESERVED_WORDS;

	std::set<uint32_t> setOfCodes;
	int numLines = 0;

	while (tempStream.getline(line, 1024))
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

		if (equalsIndex == 0)
		{
			continue;
		}

		line[equalsIndex - 1] = '\0';
		char* keyStr = line;
		char* valueStr = line + equalsIndex + 2;

		uint32_t crc = CRC::Calculate(keyStr, equalsIndex - 1);

		if (setOfCodes.find(crc) != setOfCodes.end())
		{
			std::cout << "Conflict " << keyStr << std::endl;
		}

		setOfCodes.insert(crc);
		numLines++;

		// Insert words in the string
		int i = 0;
		int lastWordIndex = 0;
		while (valueStr[i] != '\0')
		{
			if (valueStr[i] == ' ')
			{
				std::string word = std::string(&valueStr[lastWordIndex], i - lastWordIndex);
				if (words.find(word) == words.end())
				{
					words.insert({ word, currentWord });
					currentWord++;
				}

				lastWordIndex = i + 1;
				numWords++;
			}
			i++;
		}

		std::memset(line, 0, 1024);
	}

	for (auto it : words)
	{
		std::cout << it.first << " - " << it.second << std::endl;
	}

	std::cout << std::endl;
	std::cout << "Lines: " << numLines << ", Keys: " << setOfCodes.size() << std::endl;
	std::cout << "Words: " << numWords << ", Unique Words: " << words.size() << std::endl;

	delete wordBuffer;
}