#include "VSProjectGraphNode.h"

#include "Registry.h"

#include "FileUtils.h"

#include <filesystem>

VSProjectGraphNode::VSProjectGraphNode(const std::string& projectFilePath)
	: projectFilePath(projectFilePath), process(nullptr)
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
	BuildGraphNode::Start();

	std::string solutionDirectory = std::filesystem::current_path().string();
	File::EnforceForwardSlash(solutionDirectory);

	std::string msBuildPath = Registry::QueryValue("SOFTWARE\\Microsoft\\MSBuild\\4.0", "MSBuildOverrideTasksPath");
	std::string command = msBuildPath + "\\" + "msbuild.exe " + projectFilePath + " /property:Configuration=Release /property:Platform=x64 /p:SolutionDir=\"" + solutionDirectory + "/\"";

	if (process != nullptr)
	{
		delete process;
	}
	process = new BuildProcess(command);
}

void VSProjectGraphNode::Update()
{
}

bool VSProjectGraphNode::IsDone()
{
	if (process == nullptr)
	{
		return false;
	}

	return !process->StillRunning();
}