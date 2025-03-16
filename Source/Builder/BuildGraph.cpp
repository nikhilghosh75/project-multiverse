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
		if ((*it)->IsDone())
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
}

BuildState BuildGraph::GetCurrentState()
{
	return currentState;
}

void BuildGraph::AddNodeToBuild()
{
	std::vector<BuildGraphNode*> nodesToSearch;

	if (rootNode->IsDone())
	{
		nodesToSearch.push_back(rootNode);
	}

	while (nodesToSearch.size() > 0)
	{
		BuildGraphNode* node = nodesToSearch.back();
		if (!node->HasStarted() && !node->IsDone())
		{
			node->Start();
			nodesInProgress.push_back(node);
			break;
		}

		nodesToSearch.pop_back();

		if (node->IsDone())
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
		if (node->HasStarted() && !node->IsDone())
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

void BuildGraphNode::Start()
{
	hasStarted = true;
}
