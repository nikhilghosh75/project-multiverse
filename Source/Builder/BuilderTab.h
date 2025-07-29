#pragma once

#include <string>
#include <vector>

class BuilderTab
{
public:
	BuilderTab(std::string _name);

	std::string name;

	virtual void Render() = 0;
};

class BuilderTabSystem
{
public:
	static BuilderTabSystem* Get();

	void RenderTabs();

	void AddTab(BuilderTab* tab);
	void RemoveTab(BuilderTab* tab);

private:
	static BuilderTabSystem* instance;

	std::vector<BuilderTab*> tabs;
};