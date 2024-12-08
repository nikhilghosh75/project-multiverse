#pragma once

#include <optional>
#include <string>
#include <vector>

class BuildSystem
{
public:
	static BuildSystem* Get();

	void Init();

	void StartBuild();

	std::string buildFolderPath;

private:
	void BuildGame();

	class BuildConfig
	{
	public:
		std::string executablePath;
		std::string extension;
		std::string resultExtension;
		std::string folderPath;
		std::optional<std::string> projectPath;
		bool aggregateResults;
	};

	std::string ParseFilepath(const std::string& baseFilepath, bool isProject);

	std::vector<BuildConfig> configs;

	static inline BuildSystem* instance;
};