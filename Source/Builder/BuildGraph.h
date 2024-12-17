#pragma once

#include <optional>
#include <string>
#include <vector>

class BuildConfig
{
public:
	std::string executablePath;
	std::string extension;
	std::string resultExtension;
	std::string folderPath;
	std::string beforeArguments;
	std::string betweenArguments;
	std::string afterArguments;
	std::optional<std::string> projectPath;
	bool aggregateResults;
};

class BuildGraphNode
{
public:
	virtual void Start();
	virtual void Update() = 0;
	virtual bool IsDone() = 0;

	bool HasStarted() { return hasStarted; }

	std::vector<BuildGraphNode*> children;

private:
	bool hasStarted;
};

class BuildGraph
{
public:
	void Initialize(const std::string& buildFolderPath);

	void AddBuildConfig(const BuildConfig& config, const std::string& buildFolderPath);
	void AddFolderPath(const std::string& folderPath, const std::string& buildFolderPath);

	void StartBuild();

	void UpdateBuild();

private:
	static const size_t MAX_NODES_AT_ONCE = 2;

	BuildGraphNode* rootNode;

	std::vector<BuildGraphNode*> nodesInProgress;

	void AddNodeToBuild();
};