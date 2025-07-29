#pragma once

#include "BuilderTab.h"

class FileStatuses : public BuilderTab
{
public:
	FileStatuses();

	void Render() override;
};