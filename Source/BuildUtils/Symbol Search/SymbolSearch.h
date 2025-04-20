#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>

class SymbolSearch
{
public:
	struct FoundSymbol
	{
		std::string filepath;
		uint32_t line;
		uint32_t character;
	};

	struct Result
	{
		std::vector<FoundSymbol> foundSymbols;
		int filesSearched;
	};

	SymbolSearch();

	static SymbolSearch* Get();

	Result Search(const std::string& symbol, const std::vector<std::string>& projectsToSearch, const std::vector<std::string>& allowedExtensions);

private:
	static SymbolSearch* instance;

	std::optional<std::string> ParseSharpmakeFileForOutputPath(const std::string& filepath);

	void SearchFileForSymbol(const std::string& filepath, const std::string& symbol, const int* badMatchTable, Result& result);
	bool IsFileExtensionValid(const std::string& filepath, const std::vector<std::string>& allowedExtensions);

	std::map<std::string, std::string> projectsToFilepath;
};