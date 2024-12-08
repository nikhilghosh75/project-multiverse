#include "BuildSystem.h"

#include "BuildProcess.h"
#include "Registry.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>

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

		for (rapidjson::SizeType i = 0; i < d.Size(); i++)
		{
			BuildConfig config;
			if (d[i].HasMember("ProjectName"))
			{
				config.projectPath = d[i]["ProjectName"].GetString();
			}

			config.executablePath = ParseFilepath(d[i]["ExecutablePath"].GetString(), config.projectPath.has_value());
			config.extension = d[i]["Extension"].GetString();
			config.resultExtension = d[i]["ResultExtension"].GetString();
			config.folderPath = d[i]["FolderPath"].GetString();
			config.aggregateResults = d[i]["ResultAggregate"].GetBool();

			configs.push_back(config);
		}
	}

	file.close();

	buildFolderPath = "C:/Users/debgh/source/repos/Project Multiverse/Builds/Build 0.03.0";
}

void BuildSystem::StartBuild()
{
	BuildGame();
}

void BuildSystem::BuildGame()
{
	// Build the Game Project
	std::string projectFile = "Source/Game/Game.vcxproj";
	std::string msBuildPath = Registry::QueryValue("SOFTWARE\\Microsoft\\MSBuild\\4.0", "MSBuildOverrideTasksPath");
	std::string command = msBuildPath + "\\" + "msbuild.exe " + projectFile + " /property:Configuration=Release";

	BuildProcess vsProcess(command);
	while (vsProcess.StillRunning())
	{
		std::cout << "Visual Studio Running" << std::endl;
	}

	// TODO: Parse Result


	// Iterate through the configs
	for (BuildConfig& config : configs)
	{
		for (const auto& entry : std::filesystem::recursive_directory_iterator(config.folderPath))
		{
			size_t extensionPosition = entry.path().string().find(config.extension);
			if (extensionPosition != std::string::npos)
			{
			}
		}
	}
}

std::string BuildSystem::ParseFilepath(const std::string& baseFilepath, bool isProject)
{
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
				pathIndex = endPathIndex;
			}
		}
	}
	while (pathIndex != std::string::npos);

	return filepath;
}
