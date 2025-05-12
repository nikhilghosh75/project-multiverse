#pragma once
#include "BuildGraph.h"
#include "BuildProcess.h"

#include <optional>

class VSProjectGraphNode : public BuildGraphNode
{
public:
	VSProjectGraphNode(const std::string& projectFilePath);
	~VSProjectGraphNode();

	void Start() override;
	void Update() override;
	void Cancel() override;

	std::map<std::string, FileBuildState> GetFileStates() override;

private:
	void OnProjectBuildFinished(BuildProcess& buildProcess, uint32_t exitCode);

	BuildProcess* process;

	FileBuildState projectBuildState;
	std::string projectFilePath;
};