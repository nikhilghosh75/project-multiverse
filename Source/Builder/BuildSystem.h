#pragma once

#include "BuildGraph.h"

#include "DateTime.h"

#include <string>
#include <optional>
#include <vector>

struct BuildInfo
{
	std::string name;
	DateTime time;
};

class BuildSystem
{
public:
	static BuildSystem* Get();

	void Init();

	void StartBuild();

	void UpdateBuild();

	std::optional<std::string> buildFolderPath;

	std::vector<BuildInfo> previousBuilds;

private:
	std::string ParseFilepath(const std::string& baseFilepath, std::optional<std::string>& project);

	void ReportBuild();

	bool CanBuildBeStarted();

	std::vector<BuildConfig> configs;
	std::vector<std::string> foldersToCopy;

	BuildGraph buildGraph;

	bool isBuildInProgress = false;

	static inline BuildSystem* instance;
};