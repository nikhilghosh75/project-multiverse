#pragma once

#include "BuildGraph.h"

#include "DateTime.h"

#include <optional>
#include <sstream>
#include <string>
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

	void CancelBuild();

	bool IsBuildInProgress() const;

	void LogOutputFromFile(char* buffer, uint32_t nodeId);

	std::optional<std::string> buildFolderPath;

	std::vector<BuildInfo> previousBuilds;

	BuildGraph buildGraph;

	struct OutputLog
	{
		uint32_t nodeId;
		std::string filepath;
		std::string output;
	};

	std::vector<OutputLog> fileOutputLogs;
	std::stringstream output;

private:
	std::string ParseFilepath(const std::string& baseFilepath, std::optional<std::string>& project);

	void ReportBuild();

	bool CanBuildBeStarted();

	std::vector<BuildConfig> configs;
	std::vector<std::string> foldersToCopy;

	DateTime currentDatetime;

	bool isBuildInProgress = false;

	static inline BuildSystem* instance;
};