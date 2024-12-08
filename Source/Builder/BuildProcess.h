#pragma once

#include <string>

#include <Windows.h>

class BuildProcess
{
public:
	BuildProcess(std::string commandLineArgs);
	~BuildProcess();

	bool StillRunning();

private:
	PROCESS_INFORMATION pi;
	uint32_t exitValue;
};