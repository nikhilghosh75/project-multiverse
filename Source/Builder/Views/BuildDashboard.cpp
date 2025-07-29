#include "BuildDashboard.h"

#include "BuildSystem.h"

#include "imgui.h"

#include "nfd.h"

#include <filesystem>

BuildDashboard::BuildDashboard()
	: BuilderTab("Build Dashboard")
{
}

void BuildDashboard::Render()
{
	ImGui::Begin("Dashboard");

	ImGui::Text("Build Folder: %s", BuildSystem::Get()->buildFolderPath.value_or("None").c_str());
	ImGui::SameLine();
	if (ImGui::Button("Select"))
	{
		std::string currentFolderPath = BuildSystem::Get()->buildFolderPath.value_or(std::filesystem::current_path().string());

		char* outPath = nullptr;
		nfdresult_t result = NFD_PickFolder(currentFolderPath.c_str(), &outPath);

		if (result == NFD_OKAY)
		{
			BuildSystem::Get()->buildFolderPath = outPath;
		}
	}

	if (BuildSystem::Get()->buildFolderPath && !std::filesystem::is_empty(*BuildSystem::Get()->buildFolderPath))
	{
		ImGui::TextColored(ImVec4(1.0f, 1.0f, 0, 1.0f), "Warning: Build Folder is not empty");
	}

	if (BuildSystem::Get()->previousBuilds.size() == 0)
	{
		ImGui::Text("No Recent Builds");
	}
	else
	{
		BuildInfo lastBuild = BuildSystem::Get()->previousBuilds.back();
		ImGui::Text("Last Build %s", lastBuild.time.Str().c_str());
	}

	if (ImGui::Button("Build"))
	{
		BuildSystem::Get()->StartBuild();
	}

	if (BuildSystem::Get()->IsBuildInProgress())
	{
		if (ImGui::Button("Cancel"))
		{
			BuildSystem::Get()->CancelBuild();
		}
	}

	ImGui::End();
}
