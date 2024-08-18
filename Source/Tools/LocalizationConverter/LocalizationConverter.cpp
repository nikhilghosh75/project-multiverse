#include "CRC.h"
#include <fstream>
#include <iostream>
#include <set>

int main(char** argv, int argc)
{
	std::ifstream tempStream("C:/Users/debgh/source/repos/College Basketball Simuator/Data/English.txt");
	char line[1024];
	std::memset(line, 0, 1024);

	char* wordBuffer = (char*)malloc(1048576);
	int currentWordBufferPosition = 0;

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



		std::memset(line, 0, 1024);
	}

	std::cout << std::endl;
	std::cout << "Lines: " << numLines << ", Keys: " << setOfCodes.size() << std::endl;

	delete wordBuffer;
}