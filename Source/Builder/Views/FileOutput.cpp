#include "FileOutput.h"

#include "BuildSystem.h"

#include "imgui.h"

#include <sstream>

FileOutput::FileOutput(std::string _filepath)
	: BuilderTab(_filepath + " Output")
{
	filepath = _filepath;
}

void FileOutput::Render()
{
	std::string out = GetOutputOfFile();

	if (ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), ImGuiChildFlags_NavFlattened, ImGuiWindowFlags_HorizontalScrollbar))
	{
		ImGui::Text(out.c_str());
	}

	ImGui::EndChild();
}

std::string FileOutput::GetOutputOfFile()
{
	std::stringstream output;

	for (BuildSystem::OutputLog& log : BuildSystem::Get()->fileOutputLogs)
	{
		if (log.filepath == filepath)
		{
			output << log.output;
		}
	}

	return output.str();
}
