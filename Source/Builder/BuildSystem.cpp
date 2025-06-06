#include "BuildSystem.h"

#include "BuildProcess.h"
#include "FileUtils.h"
#include "Registry.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#include "rapidjson/document.h"

BuildSystem* BuildSystem::Get()
{
	if (instance == nullptr)
	{
		instance = new BuildSystem();
	}

	return instance;
}

void BuildSystem::Init()
{
	// Read buildconfig.json
	std::size_t size = std::filesystem::file_size("buildconfig.json");

	std::ifstream file("buildconfig.json");
	file.seekg(0, std::ios::beg);

	std::vector<char> buffer(size);
	file.read(buffer.data(), size);

	if (size != 0)
	{
		rapidjson::Document d;
		d.Parse(buffer.data());

		rapidjson::Value& steps = d["Steps"];

		for (rapidjson::SizeType i = 0; i < steps.Size(); i++)
		{
			BuildConfig config;
			if (steps[i].HasMember("ProjectPath"))
			{
				config.projectPath = steps[i]["ProjectPath"].GetString();
			}

			config.executablePath = ParseFilepath(steps[i]["ExecutablePath"].GetString(), config.projectPath);
			config.extension = steps[i]["Extension"].GetString();
			config.resultExtension = steps[i]["ResultExtension"].GetString();
			config.folderPath = steps[i]["FolderPath"].GetString();
			config.aggregateResults = steps[i]["ResultAggregate"].GetBool();

			if (steps[i].HasMember("FlagsBefore"))
			{
				config.beforeArguments = steps[i]["FlagsBefore"].GetString();
			}
			else
			{
				config.beforeArguments = "";
			}

			if (steps[i].HasMember("FlagsBetween"))
			{
				config.betweenArguments = steps[i]["FlagsBetween"].GetString();
			}
			else
			{
				config.betweenArguments = "";
			}

			if (steps[i].HasMember("FlagsAfter"))
			{
				config.afterArguments = steps[i]["FlagsAfter"].GetString();
			}
			else
			{
				config.afterArguments = "";
			}

			configs.push_back(config);
		}

		rapidjson::Value& copyFolders = d["FoldersToCopy"];
		for (rapidjson::SizeType i = 0; i < copyFolders.Size(); i++)
		{
			foldersToCopy.push_back(copyFolders[i].GetString());
		}
	}

	file.close();
}

void BuildSystem::StartBuild()
{
	if (!CanBuildBeStarted())
	{
		return;
	}

	buildGraph.Initialize(*buildFolderPath);
	for (BuildConfig& config : configs)
	{
		buildGraph.AddBuildConfig(config, *buildFolderPath);
	}

	for (std::string& folderPath : foldersToCopy)
	{
		buildGraph.AddFolderPath(folderPath, *buildFolderPath);
	}

	std::filesystem::create_directory(*buildFolderPath);
	std::filesystem::create_directory(*buildFolderPath + "/Data");

	buildGraph.StartBuild();

	isBuildInProgress = true;
}

void BuildSystem::UpdateBuild()
{
	if (isBuildInProgress)
	{
		buildGraph.UpdateBuild();

		BuildState state = buildGraph.GetCurrentState();
		if (state == BuildState::Complete)
		{
			isBuildInProgress = false;
			ReportBuild();
		}
		else if (state == BuildState::Failed)
		{
			isBuildInProgress = false;
		}
	}
}

void BuildSystem::CancelBuild()
{
	if (isBuildInProgress)
	{
		buildGraph.CancelBuild();
		isBuildInProgress = false;
	}
}

bool BuildSystem::IsBuildInProgress() const
{
	return isBuildInProgress;
}

std::string BuildSystem::ParseFilepath(const std::string& baseFilepath, std::optional<std::string>& projectPath)
{
	if (projectPath)
	{
		return projectPath.value() + "/" + baseFilepath;
	}

	std::string filepath = "";
	int pathIndex = 0;
	int endPathIndex = 0;
	do
	{
		int nextPathIndex = baseFilepath.find("{{", endPathIndex);
		filepath += baseFilepath.substr(pathIndex, nextPathIndex - pathIndex);

		pathIndex = nextPathIndex;
		if (pathIndex != std::string::npos)
		{
			endPathIndex = baseFilepath.find("}}", pathIndex);
			std::string envVariable = baseFilepath.substr(pathIndex + 2, endPathIndex - pathIndex - 2);

			char* envValue = std::getenv(envVariable.c_str());
			if (envValue != nullptr)
			{
				filepath += envValue;
				pathIndex = endPathIndex + 2;
			}
		}
	}
	while (pathIndex != std::string::npos);

	File::EnforceForwardSlash(filepath);

	return filepath;
}

void BuildSystem::ReportBuild()
{
	std::cout << "BUILD COMPLETE";

	BuildInfo info;
	info.name = "";
	info.time = DateTime::Now();

	previousBuilds.push_back(info);
}

bool BuildSystem::CanBuildBeStarted()
{
	if (!buildFolderPath.has_value())
	{
		return false;
	}
	return true;
}
