#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>

enum class BuildState
{
	NotStarted,
	InProgress,
	Complete,
	Failed
};

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

enum class FileBuildState
{
	NotStarted,
	InProgress,
	Succeeded,
	Failed
};

enum class NodeState
{
	NotStarted,
	InProgress,
	Succeeded,
	Failed
};

class BuildGraphNode
{
public:
	virtual void Start() = 0;
	virtual void Update() = 0;

	virtual void Cancel() = 0;

	virtual std::map<std::string, FileBuildState> GetFileStates() = 0;

	NodeState GetState() inline const { return state; }

	std::vector<BuildGraphNode*> children;

protected:
	NodeState state = NodeState::NotStarted;
};

class BuildGraph
{
public:
	void Initialize(const std::string& buildFolderPath);

	void AddBuildConfig(const BuildConfig& config, const std::string& buildFolderPath);
	void AddFolderPath(const std::string& folderPath, const std::string& buildFolderPath);

	void StartBuild();

	void UpdateBuild();

	void CancelBuild();

	BuildState GetCurrentState();

	std::map<std::string, FileBuildState> GetFileStates();

private:
	static const size_t MAX_NODES_AT_ONCE = 2;

	BuildGraphNode* rootNode;

	std::vector<BuildGraphNode*> nodesInProgress;

	void AddNodeToBuild();
	bool IsBuildComplete();
	bool DidBuildFail();

	BuildState currentState;
};