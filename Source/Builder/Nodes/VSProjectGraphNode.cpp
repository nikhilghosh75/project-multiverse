#include "VSProjectGraphNode.h"

#include "Registry.h"

#include "FileUtils.h"

#include <filesystem>

VSProjectGraphNode::VSProjectGraphNode(const std::string& projectFilePath)
	: projectFilePath(projectFilePath), process(nullptr), projectBuildState(FileBuildState::NotStarted)
{

}

VSProjectGraphNode::~VSProjectGraphNode()
{
	if (process != nullptr)
	{
		delete process;
	}
}

void VSProjectGraphNode::Start()
{
	std::string solutionDirectory = std::filesystem::current_path().string();
	File::EnforceForwardSlash(solutionDirectory);

	std::string msBuildPath = Registry::QueryValue("SOFTWARE\\Microsoft\\MSBuild\\4.0", "MSBuildOverrideTasksPath");
	std::string command = msBuildPath + "\\" + "msbuild.exe " + projectFilePath + " /property:Configuration=Release /property:Platform=x64 /p:SolutionDir=\"" + solutionDirectory + "/\"";

	if (process != nullptr)
	{
		delete process;
	}
	process = new BuildProcess(command, std::bind(&VSProjectGraphNode::OnProjectBuildFinished, this, std::placeholders::_1, std::placeholders::_2));

	projectBuildState = FileBuildState::InProgress;
	state = NodeState::InProgress;
}

void VSProjectGraphNode::Update()
{
	process->Update();
}

void VSProjectGraphNode::Cancel()
{
	if (process != nullptr)
	{
		process->Terminate();
	}
}

std::map<std::string, FileBuildState> VSProjectGraphNode::GetFileStates()
{
	return { { projectFilePath, projectBuildState} };
}

void VSProjectGraphNode::OnProjectBuildFinished(BuildProcess& buildProcess, uint32_t exitCode)
{
	if (exitCode == 0)
	{
		projectBuildState = FileBuildState::Succeeded;
		state = NodeState::Succeeded;
	}
	else
	{
		projectBuildState = FileBuildState::Failed;
		state = NodeState::Failed;
	}
}
