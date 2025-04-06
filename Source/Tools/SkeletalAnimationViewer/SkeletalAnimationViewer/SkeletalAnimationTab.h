#pragma once

#include <functional>
#include <string>
#include <vector>

class SkeletalAnimationTab
{
public:
	SkeletalAnimationTab(std::string _tabName, bool _isClosable);

	std::string tabName;
	bool isClosable;

	std::function<void()> onTabClose;

	virtual void Render() = 0;
};

class SkeletalAnimationTabSystem
{
public:
	static SkeletalAnimationTabSystem* Get();

	void RenderTabs();

	void AddTab(SkeletalAnimationTab* tab);
	void RemoveTab(SkeletalAnimationTab* tab);

private:
	static SkeletalAnimationTabSystem* instance;

	std::vector<SkeletalAnimationTab*> tabs;
};