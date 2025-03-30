#pragma once

#include "BuildGraph.h"

#include <string>

class CopyFolderGraphNode : public BuildGraphNode
{
public:
	CopyFolderGraphNode(const std::string& folderPath, const std::string& buildFolderPath);

	void Start() override;
	void Update() override;
	bool IsDone() override;

	std::map<std::string, FileBuildState> GetFileStates() override;

private:
	std::string folderPath;
	std::string buildFolderPath;

};