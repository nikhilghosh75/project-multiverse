#include "CopyVSGraphNode.h"

#include <filesystem>


CopyVSGraphNode::CopyVSGraphNode(const std::string& projectFilePath, const std::string& buildFolderPath)
	: projectFilePath(projectFilePath), buildFolderPath(buildFolderPath)
{
}

void CopyVSGraphNode::Start()
{
	const std::string platform = "x64";
	const std::string config = "Release";

	size_t slashIndex = projectFilePath.find_last_of('/');
	size_t periodIndex = projectFilePath.find('.');
	std::string exeName = projectFilePath.substr(slashIndex + 1, periodIndex - slashIndex - 1);

	std::string exeFilepath = platform + "/" + config + "/" + exeName + ".exe";
	std::string resultFilepath = buildFolderPath + "/" + exeName + ".exe";

	if (std::filesystem::exists(exeFilepath))
	{
		std::filesystem::copy_file(exeFilepath, resultFilepath, std::filesystem::copy_options::overwrite_existing);
		state = NodeState::Succeeded;
	}
	else
	{
		state = NodeState::Failed;
	}
}

void CopyVSGraphNode::Update()
{
}

void CopyVSGraphNode::Cancel()
{
}

std::map<std::string, FileBuildState> CopyVSGraphNode::GetFileStates()
{
	FileBuildState fileState = state == NodeState::NotStarted ? FileBuildState::NotStarted : 
		(state == NodeState::Succeeded ? FileBuildState::Succeeded : FileBuildState::Failed);

	return { {projectFilePath, fileState } };
}
