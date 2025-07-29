#include "AssetGroupGraphNode.h"

#include "FileUtils.h"

#include <filesystem>

AssetGroupGraphNode::AssetGroupGraphNode(const BuildConfig& config, const std::string& buildFolderPath)
	: config(config), process(nullptr), currentFileIndex(0), buildFolderPath(buildFolderPath)
{
	for (const auto& entry : std::filesystem::recursive_directory_iterator(config.folderPath))
	{
		size_t extensionPosition = entry.path().string().find(config.extension);
		if (extensionPosition != std::string::npos)
		{
			std::string filepath = entry.path().string();
			File::EnforceForwardSlash(filepath);
			files.push_back(filepath);
		}
	}
}

AssetGroupGraphNode::~AssetGroupGraphNode()
{
	if (process != nullptr)
	{
		delete process;
	}
}

void AssetGroupGraphNode::Start()
{
	std::string resultFolderPath = GenerateResultForFile(files[0]);
	resultFolderPath = resultFolderPath.substr(0, resultFolderPath.find_last_of('/'));
	std::filesystem::create_directories(resultFolderPath);

	currentFileIndex = 0;

	std::string command = GenerateCommandForFile(files[0]);

	process = new BuildProcess(command, id);
	state = NodeState::InProgress;
}

void AssetGroupGraphNode::Update()
{
	if (process && !process->StillRunning())
	{
		delete process;
		process = nullptr;

		currentFileIndex++;
		if (currentFileIndex < files.size())
		{
			std::string command = GenerateCommandForFile(files[currentFileIndex]);
			process = new BuildProcess(command);
		}
		else
		{
			state = NodeState::Succeeded;
		}
	}
}

void AssetGroupGraphNode::Cancel()
{
	if (process != nullptr)
	{
		process->Terminate();
		delete process;
		process = nullptr;
	}
}

std::map<std::string, FileBuildState> AssetGroupGraphNode::GetFileStates()
{
	std::map<std::string, FileBuildState> fileStates;

	for (int i = 0; i < files.size(); i++)
	{
		if (state == NodeState::NotStarted)
		{
			fileStates.insert({ files[i], FileBuildState::NotStarted });
		}
		else if (i < currentFileIndex)
		{
			if (std::find(failedFileIndices.begin(), failedFileIndices.end(), i) != failedFileIndices.end())
			{
				fileStates.insert({ files[i], FileBuildState::Failed });
			}
			else
			{
				fileStates.insert({ files[i], FileBuildState::Succeeded });
			}
		}
		else if (i == currentFileIndex)
		{
			fileStates.insert({ files[i], FileBuildState::InProgress });
		}
		else
		{
			fileStates.insert({ files[i], FileBuildState::NotStarted });
		}
	}

	return fileStates;
}

void AssetGroupGraphNode::OnProjectBuildFinished(BuildProcess& buildProcess, uint32_t exitCode)
{
	if (exitCode != 0)
	{
		failedFileIndices.push_back(currentFileIndex);
	}
}

std::string AssetGroupGraphNode::GenerateCommandForFile(const std::string& filepath) const
{
	std::string command = "";
	std::string resultFilepath = GenerateResultForFile(filepath);

	if (resultFilepath.find(' ') != std::string::npos)
	{
		resultFilepath = "\"" + resultFilepath + "\"";
	}


	command += config.executablePath + " ";

	if (!config.beforeArguments.empty())
	{
		command += config.beforeArguments + " ";
	}

	command += filepath + " ";

	if (!config.betweenArguments.empty())
	{
		command += config.betweenArguments + " ";
	}

	command += resultFilepath;

	if (!config.afterArguments.empty())
	{
		command += " " + config.afterArguments;
	}

	return command;
}

std::string AssetGroupGraphNode::GenerateResultForFile(const std::string& filepath) const
{
	size_t extensionIndex = filepath.find(config.extension.c_str());
	size_t slashIndex = filepath.find("Data/");
	std::string resultFilepath = buildFolderPath + "/" + filepath.substr(slashIndex, extensionIndex) + config.resultExtension;

	return resultFilepath;
}
