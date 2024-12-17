#pragma once
#include "BuildGraph.h"
#include "BuildProcess.h"

#include <optional>

class VSProjectGraphNode : public BuildGraphNode
{
public:
	VSProjectGraphNode(const std::string& projectFilePath);
	~VSProjectGraphNode();

	void Start() override;
	void Update() override;
	bool IsDone() override;

private:
	BuildProcess* process;

	std::string projectFilePath;
};