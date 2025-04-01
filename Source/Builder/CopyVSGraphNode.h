#pragma once

#include "BuildGraph.h"

#include <string>

class CopyVSGraphNode : public BuildGraphNode
{
public:
	CopyVSGraphNode(const std::string& projectFilePath, const std::string& buildFolderPath);

	void Start() override;
	void Update() override;
	bool IsDone() override;

	std::map<std::string, FileBuildState> GetFileStates() override;

private:
	std::string projectFilePath;
	
	std::string buildFolderPath;
};