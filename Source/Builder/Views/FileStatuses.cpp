#include "FileStatuses.h"

#include "BuildSystem.h"
#include "BuilderTab.h"
#include "Views/FileOutput.h"

#include "imgui.h"

FileStatuses::FileStatuses()
	: BuilderTab("File Statuses")
{
}

void FileStatuses::Render()
{
	if (ImGui::BeginTable("Files", 3))
	{
		std::map<std::string, FileBuildState> fileStates = BuildSystem::Get()->buildGraph.GetFileStates();

		for (std::pair<std::string, FileBuildState> it : fileStates)
		{
			ImGui::TableNextRow();

			ImGui::TableNextColumn();
			ImGui::Text(it.first.c_str());

			ImGui::TableNextColumn();

			ImVec4 color;
			switch (it.second)
			{
			case FileBuildState::NotStarted:
				ImGui::TextColored(ImVec4(0.85f, 0.85f, 0.85f, 1.0f), "Not Started");
				break;
			case FileBuildState::InProgress:
				ImGui::TextColored(ImVec4(0.15f, 0.65f, 0.f, 1.0f), "In Progress");
				break;
			case FileBuildState::Succeeded:
				ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Succeeded");
				break;
			case FileBuildState::Failed:
				ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Failed");
				break;
			}

			ImGui::TableNextColumn();

			if (it.second != FileBuildState::NotStarted)
			{
				if (ImGui::Button("View Output"))
				{
					BuilderTabSystem::Get()->AddTab(new FileOutput(it.first));
				}
			}
		}

		ImGui::EndTable();
	}
}
