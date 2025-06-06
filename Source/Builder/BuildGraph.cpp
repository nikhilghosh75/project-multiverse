#include "BuildGraph.h"

#include "AssetGroupGraphNode.h"
#include "CopyFolderGraphNode.h"
#include "CopyVSGraphNode.h"
#include "VSProjectGraphNode.h"

void BuildGraph::Initialize(const std::string& buildFolderPath)
{
	rootNode = new VSProjectGraphNode("Source/Game/Game.vcxproj");
	rootNode->children.push_back(new CopyVSGraphNode("Source/Game/Game.vcxproj", buildFolderPath));

	currentState = BuildState::NotStarted;
}

void BuildGraph::AddBuildConfig(const BuildConfig& config, const std::string& buildFolderPath)
{
	BuildGraphNode* currentNode = rootNode;

	if (config.projectPath.has_value())
	{
		currentNode->children.push_back(new VSProjectGraphNode(config.projectPath.value()));
		currentNode = currentNode->children.back();
	}

	currentNode->children.push_back(new AssetGroupGraphNode(config, buildFolderPath));
}

void BuildGraph::AddFolderPath(const std::string& folderPath, const std::string& buildFolderPath)
{
	rootNode->children.push_back(new CopyFolderGraphNode(folderPath, buildFolderPath));
}

void BuildGraph::StartBuild()
{
	rootNode->Start();
	nodesInProgress.push_back(rootNode);

	currentState = BuildState::InProgress;
}

void BuildGraph::UpdateBuild()
{
	for (BuildGraphNode* node : nodesInProgress)
	{
		node->Update();
	}

	for (auto it = nodesInProgress.begin(); it != nodesInProgress.end(); ++it)
	{
		NodeState state = (*it)->GetState();
		if (state == NodeState::Succeeded || state == NodeState::Failed)
		{
			nodesInProgress.erase(it);
			break;
		}
	}

	// Add nodes if we can
	bool roomToAddNodes = nodesInProgress.size() < MAX_NODES_AT_ONCE;
	if (roomToAddNodes)
	{
		AddNodeToBuild();
	}

	if (IsBuildComplete())
	{
		currentState = BuildState::Complete;
	}

	if (DidBuildFail())
	{
		currentState = BuildState::Failed;
	}
}

void BuildGraph::CancelBuild()
{
	std::vector<BuildGraphNode*> nodesToSearch;
	nodesToSearch.push_back(rootNode);

	while (nodesToSearch.size() > 0)
	{
		BuildGraphNode* node = nodesToSearch.back();
		if (node->GetState() == NodeState::InProgress)
		{
			node->Cancel();
		}

		nodesToSearch.pop_back();

		for (int i = 0; i < node->children.size(); i++)
		{
			nodesToSearch.push_back(node->children[i]);
		}
	}
}

BuildState BuildGraph::GetCurrentState()
{
	return currentState;
}

std::map<std::string, FileBuildState> BuildGraph::GetFileStates()
{
	std::map<std::string, FileBuildState> fileStates;

	std::vector<BuildGraphNode*> nodesToSearch;
	nodesToSearch.push_back(rootNode);

	while (nodesToSearch.size() > 0)
	{
		BuildGraphNode* node = nodesToSearch.back();
		if (node != nullptr)
		{
			std::map<std::string, FileBuildState> nodeFileStates = node->GetFileStates();
			for (std::pair<std::string, FileBuildState> it : nodeFileStates)
			{
				fileStates.insert(it);
			}

			nodesToSearch.pop_back();

			for (int i = 0; i < node->children.size(); i++)
			{
				nodesToSearch.push_back(node->children[i]);
			}
		}
		else
		{
			nodesToSearch.pop_back();
		}
	}

	return fileStates;
}

void BuildGraph::AddNodeToBuild()
{
	std::vector<BuildGraphNode*> nodesToSearch;

	if (rootNode->GetState() == NodeState::Succeeded)
	{
		nodesToSearch.push_back(rootNode);
	}

	while (nodesToSearch.size() > 0)
	{
		BuildGraphNode* node = nodesToSearch.back();
		if (node->GetState() == NodeState::NotStarted)
		{
			node->Start();
			nodesInProgress.push_back(node);
			break;
		}

		nodesToSearch.pop_back();

		if (node->GetState() == NodeState::Succeeded)
		{
			for (int i = 0; i < node->children.size(); i++)
			{
				nodesToSearch.push_back(node->children[i]);
			}
		}
	}
}

bool BuildGraph::IsBuildComplete()
{
	std::vector<BuildGraphNode*> nodesToSearch;
	nodesToSearch.push_back(rootNode);
	bool isAnyNodeNotDone = false;

	while (nodesToSearch.size() > 0)
	{
		BuildGraphNode* node = nodesToSearch.back();
		if (node->GetState() == NodeState::InProgress)
		{
			isAnyNodeNotDone = true;
		}

		nodesToSearch.pop_back();

		for (int i = 0; i < node->children.size(); i++)
		{
			nodesToSearch.push_back(node->children[i]);
		}
	}

	return !isAnyNodeNotDone;
}

bool BuildGraph::DidBuildFail()
{
	std::vector<BuildGraphNode*> nodesToSearch;
	nodesToSearch.push_back(rootNode);
	bool didAnyNodeFail = false;

	while (nodesToSearch.size() > 0)
	{
		BuildGraphNode* node = nodesToSearch.back();
		if (node->GetState() == NodeState::Failed)
		{
			didAnyNodeFail = true;
		}

		nodesToSearch.pop_back();

		for (int i = 0; i < node->children.size(); i++)
		{
			nodesToSearch.push_back(node->children[i]);
		}
	}

	return didAnyNodeFail;
}

