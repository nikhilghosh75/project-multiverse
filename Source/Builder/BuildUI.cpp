#include "BuildUI.h"

#include "BuildSystem.h"

#include "imgui.h"

BuildUI::BuildUI()
{

}

BuildUI::~BuildUI()
{
}

void BuildUI::Render()
{
	ImGui::Begin("Dashboard");

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

	ImGui::End();
}
