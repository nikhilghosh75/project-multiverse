#include "TotalOutput.h"

#include "BuildSystem.h"

#include "imgui.h"

TotalOutput::TotalOutput()
	: BuilderTab("Output")
{
}

void TotalOutput::Render()
{
	if (ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), ImGuiChildFlags_NavFlattened, ImGuiWindowFlags_HorizontalScrollbar))
	{
		std::string totalOutput = BuildSystem::Get()->output.str();

		ImGui::Text(totalOutput.c_str());
	}

	ImGui::EndChild();
}
