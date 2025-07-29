#include "BuilderTab.h"

#include "imgui.h"

#include <optional>

BuilderTabSystem* BuilderTabSystem::instance;

BuilderTab::BuilderTab(std::string _name)
	: name(_name)
{
}

BuilderTabSystem* BuilderTabSystem::Get()
{
	if (instance == nullptr)
	{
		instance = new BuilderTabSystem();
	}

	return instance;
}

void BuilderTabSystem::RenderTabs()
{
	std::optional<int> indexToRemove = std::nullopt;
	for (int i = 0; i < tabs.size(); i++)
	{
		bool isOpen = true;
		if (ImGui::Begin(tabs[i]->name.c_str(), &isOpen))
		{
			tabs[i]->Render();
		}

		ImGui::End();

		if (!isOpen)
		{
			indexToRemove = i;
		}
	}

	if (indexToRemove)
	{
		tabs.erase(tabs.begin() + *indexToRemove);
	}
}

void BuilderTabSystem::AddTab(BuilderTab* tab)
{
	tabs.push_back(tab);
}

void BuilderTabSystem::RemoveTab(BuilderTab* tab)
{
	tabs.erase(std::remove(tabs.begin(), tabs.end(), tab));
}
