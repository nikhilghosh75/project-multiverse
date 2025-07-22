#include "CopyFolderGraphNode.h"

#include <filesystem>

#include "FileUtils.h"

CopyFolderGraphNode::CopyFolderGraphNode(const std::string& folderPath, const std::string& buildFolderPath)
	: folderPath(folderPath), buildFolderPath(buildFolderPath)
{
	
}

void CopyFolderGraphNode::Start()
{
	if (std::filesystem::exists(folderPath))
	{
		std::string resultFolderPath = buildFolderPath + "/" + folderPath;
		std::filesystem::copy(folderPath, resultFolderPath, std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive);

		state = NodeState::Succeeded;
	}
	else
	{
		state = NodeState::Failed;
	}
}

void CopyFolderGraphNode::Update()
{
}

void CopyFolderGraphNode::Cancel()
{
}

std::map<std::string, FileBuildState> CopyFolderGraphNode::GetFileStates()
{
	return std::map<std::string, FileBuildState>();
}
