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
	ImGui::Begin("Test");

	if (ImGui::Button("Build"))
	{
		BuildSystem::Get()->StartBuild();
	}

	ImGui::End();
}
