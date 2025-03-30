#include "CopyVSGraphNode.h"

#include <filesystem>


CopyVSGraphNode::CopyVSGraphNode(const std::string& projectFilePath, const std::string& buildFolderPath)
	: projectFilePath(projectFilePath), buildFolderPath(buildFolderPath)
{
}

void CopyVSGraphNode::Start()
{
	BuildGraphNode::Start();

	const std::string platform = "x64";
	const std::string config = "Release";

	size_t slashIndex = projectFilePath.find_last_of('/');
	size_t periodIndex = projectFilePath.find('.');
	std::string exeName = projectFilePath.substr(slashIndex + 1, periodIndex - slashIndex - 1);

	std::string exeFilepath = platform + "/" + config + "/" + exeName + ".exe";
	std::string resultFilepath = buildFolderPath + "/" + exeName + ".exe";

	std::filesystem::copy_file(exeFilepath, resultFilepath, std::filesystem::copy_options::overwrite_existing);
}

void CopyVSGraphNode::Update()
{
}

bool CopyVSGraphNode::IsDone()
{
	return HasStarted();
}

std::map<std::string, FileBuildState> CopyVSGraphNode::GetFileStates()
{
	return { {projectFilePath, HasStarted() ? FileBuildState::Succeeded : FileBuildState::NotStarted } };
}
