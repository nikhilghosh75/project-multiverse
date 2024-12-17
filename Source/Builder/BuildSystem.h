#pragma once

#include "BuildGraph.h"

#include <optional>
#include <vector>

class BuildSystem
{
public:
	static BuildSystem* Get();

	void Init();

	void StartBuild();

	void UpdateBuild();

	std::string buildFolderPath;

private:
	std::string ParseFilepath(const std::string& baseFilepath, std::optional<std::string>& project);

	std::vector<BuildConfig> configs;
	std::vector<std::string> foldersToCopy;

	BuildGraph buildGraph;

	bool isBuildInProgress = false;

	static inline BuildSystem* instance;
};