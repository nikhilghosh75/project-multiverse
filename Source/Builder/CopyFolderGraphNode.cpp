#include "CopyFolderGraphNode.h"

#include <filesystem>

#include "FileUtils.h"

CopyFolderGraphNode::CopyFolderGraphNode(const std::string& folderPath, const std::string& buildFolderPath)
	: folderPath(folderPath), buildFolderPath(buildFolderPath)
{
	
}

void CopyFolderGraphNode::Start()
{
	BuildGraphNode::Start();

	std::string resultFolderPath = buildFolderPath + "/" + folderPath;
	std::filesystem::copy(folderPath, resultFolderPath, std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive);
}

void CopyFolderGraphNode::Update()
{
}

bool CopyFolderGraphNode::IsDone()
{
	return HasStarted();
}
