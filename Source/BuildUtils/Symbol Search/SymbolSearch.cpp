#include "SymbolSearch.h"

#include "MathUtils.h"

#include <filesystem>
#include <fstream>

SymbolSearch* SymbolSearch::instance = nullptr;

SymbolSearch::SymbolSearch()
{
	// Search the sharpmake directory for the projects and their source paths
	const std::string sharpmakeDirectory = "Make";

	for (auto it : std::filesystem::directory_iterator(sharpmakeDirectory))
	{
		if (it.path().string().find("sharpmake.cs") != std::string::npos)
		{
			std::string projectName = it.path().stem().stem().string();

			std::optional<std::string> sourcePath = ParseSharpmakeFileForOutputPath(it.path().string());
			if (sourcePath && (*sourcePath).size() > 0)
			{
				projectsToFilepath.insert({ projectName, *sourcePath });
			}
		}
	}
}

SymbolSearch* SymbolSearch::Get()
{
	if (instance == nullptr)
	{
		instance = new SymbolSearch();
	}
	return instance;
}

std::optional<std::string> SymbolSearch::ParseSharpmakeFileForOutputPath(const std::string& filepath)
{
	// This parses sharpmake files that have one line that ": ProjectCommon" and one line that 
	// references a source project path in the Source path
	// TODO: Configure this to better serve more complex Sharpmake projects
	bool isActualProject = false;
	std::string sourcePath = "";

	std::ifstream inFile(filepath);
	std::string line;
	line.reserve(256);
	while (std::getline(inFile, line))
	{
		if (line.find(": ProjectCommon") != std::string::npos)
		{
			isActualProject = true;
		}

		if (line.find("SourceRootPath =") != std::string::npos)
		{
			size_t foundPosition = line.find("[project.SharpmakeCsPath]/../");
			sourcePath = line.substr(foundPosition + 29, line.size() - foundPosition - 31);
		}
	}

	return isActualProject ? std::optional<std::string>(sourcePath) : std::nullopt;
}

void SymbolSearch::SearchFileForSymbol(const std::string& filepath, const std::string& symbol, const int* badMatchTable, Result& result)
{
	std::filesystem::path path(filepath);
	std::size_t size = std::filesystem::file_size(path);

	std::ifstream file(filepath, std::ios::binary | std::ios::ate);
	file.seekg(0, std::ios::beg);

	int currentLineNumber = 0;

	std::vector<char> buffer(size);
	std::vector<int> foundMatchPositions;
	if (file.read(buffer.data(), size))
	{
		result.filesSearched++;
		int currentIndex = symbol.size() - 1;
		while (currentIndex < size)
		{
			bool isMatch = true;
			for (int i = 0; i < symbol.size(); i++)
			{
				char bufferChar = buffer[currentIndex - i];
				char symbolChar = symbol[symbol.size() - i - 1];
				if (bufferChar != symbolChar)
				{
					isMatch = false;
					currentIndex += badMatchTable[buffer[currentIndex - i]];
					break;
				}
			}

			if (isMatch)
			{
				foundMatchPositions.push_back(currentIndex - (symbol.size() - 1));
				currentIndex += 1;
			}
		}

		if (foundMatchPositions.size() > 0)
		{
			int currentMatchPositionIndex = 0;
			int lastLinePosition = 0;
			int currentLine = 1;
			for (int i = 0; i < size; i++)
			{
				if (buffer[i] == '\n')
				{
					int currentMatchPosition = foundMatchPositions[currentMatchPositionIndex];
					if (currentMatchPosition > lastLinePosition && currentMatchPosition <= i)
					{
						FoundSymbol foundSymbol = { filepath, currentLine, currentMatchPosition - lastLinePosition };
						result.foundSymbols.push_back(foundSymbol);
						currentMatchPositionIndex++;

						if (currentMatchPositionIndex >= foundMatchPositions.size())
						{
							break;
						}
					}

					currentLine++;
					lastLinePosition = i;
				}
			}
		}
	}
}

bool SymbolSearch::IsFileExtensionValid(const std::string& filepath, const std::vector<std::string>& allowedExtensions)
{
	for (int i = 0; i < allowedExtensions.size(); i++)
	{
		if (filepath.find(allowedExtensions[i]) != std::string::npos)
		{
			return true;
		}
	}
	return false;
}

SymbolSearch::Result SymbolSearch::Search(const std::string& symbol, const std::vector<std::string>& projectsToSearch, const std::vector<std::string>& allowedExtensions)
{
	Result result;
	result.filesSearched = 0;

	// Implement simple boyer-moore algorithm
	int badCharTable[128];
	for (int i = 0; i < 128; i++)
	{
		badCharTable[i] = symbol.size();
	}
	for (int i = 0; i < symbol.size(); i++)
	{
		badCharTable[symbol[i]] = Math::Max(1, symbol.size() - i - 1);
	}

	for (const std::string& projectName : projectsToSearch)
	{
		auto foundProject = projectsToFilepath.find(projectName);
		if (foundProject != projectsToFilepath.end())
		{
			std::string folderPath = (*foundProject).second;
			for (auto it : std::filesystem::recursive_directory_iterator(folderPath))
			{
				if (IsFileExtensionValid(it.path().string(), allowedExtensions))
				{
					SearchFileForSymbol(it.path().string(), symbol, badCharTable, result);
				}
			}
		}
	}

	return result;
}
