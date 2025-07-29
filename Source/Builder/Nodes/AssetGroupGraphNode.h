#pragma once
#include "BuildGraph.h"
#include "BuildProcess.h"

class AssetGroupGraphNode : public BuildGraphNode
{
public:
	AssetGroupGraphNode(const BuildConfig& config, const std::string& buildFolderPath);
	~AssetGroupGraphNode();

	void Start() override;
	void Update() override;
	void Cancel() override;

	std::map<std::string, FileBuildState> GetFileStates() override;

private:
	void OnProjectBuildFinished(BuildProcess& buildProcess, uint32_t exitCode);

	BuildProcess* process;

	BuildConfig config;

	std::vector<std::string> files;
	std::vector<int> failedFileIndices;
	int currentFileIndex;

	std::string buildFolderPath;

	std::string GenerateCommandForFile(const std::string& filepath) const;
	std::string GenerateResultForFile(const std::string& filepath) const;
};