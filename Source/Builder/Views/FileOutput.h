#pragma once

#include "BuilderTab.h"

class FileOutput : public BuilderTab
{
public:
	FileOutput(std::string filepath);

	void Render() override;

private:
	std::string GetOutputOfFile();

	std::string filepath;
};