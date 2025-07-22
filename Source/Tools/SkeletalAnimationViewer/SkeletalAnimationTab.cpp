#include "SkeletalAnimationTab.h"

#include "imgui.h"

#include <optional>

SkeletalAnimationTab::SkeletalAnimationTab(std::string _tabName, bool _isClosable)
	: tabName(_tabName), isClosable(_isClosable)
{
}

SkeletalAnimationTabSystem* SkeletalAnimationTabSystem::instance = nullptr;

SkeletalAnimationTabSystem* SkeletalAnimationTabSystem::Get()
{
	if (instance == nullptr)
	{
		instance = new SkeletalAnimationTabSystem();
	}

	return instance;
}

void SkeletalAnimationTabSystem::RenderTabs()
{
	std::optional<int> indexToRemove = std::nullopt;
	for (int i = 0; i < tabs.size(); i++)
	{
		if (tabs[i]->isClosable)
		{
			bool isOpen = true;
			if (ImGui::Begin(tabs[i]->tabName.c_str(), &isOpen))
			{
				tabs[i]->Render();
			}

			ImGui::End();

			if (!isOpen)
			{
				indexToRemove = i;
			}
		}
		else
		{
			ImGui::Begin(tabs[i]->tabName.c_str());
			tabs[i]->Render();
			ImGui::End();
		}
	}

	if (indexToRemove)
	{
		tabs[*indexToRemove]->onTabClose();
		tabs.erase(tabs.begin() + *indexToRemove);
	}
}

void SkeletalAnimationTabSystem::AddTab(SkeletalAnimationTab* tab)
{
	tabs.push_back(tab);
}

void SkeletalAnimationTabSystem::RemoveTab(SkeletalAnimationTab* tab)
{
	// TODO: This doesn't delete the tab pointer because the tabs system doesn't necessarily own the pointer
	// Could probably be fixed with shared pointers if necessary, but tabs tend to be lightweight so we don't care about leaking
	tabs.erase(std::remove(tabs.begin(), tabs.end(), tab));
}

