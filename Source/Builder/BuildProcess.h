#pragma once

#include <functional>
#include <optional>
#include <string>
#include <sstream>

#include <Windows.h>

class BuildProcess
{
public:
	BuildProcess(std::string commandLineArgs);
	BuildProcess(std::string commandLineArgs, std::function<void(BuildProcess&, uint32_t)>);
	~BuildProcess();

	std::string GetOutput() const;

	void Update();

	bool StillRunning();

private:
	std::stringstream outputStream;

	PROCESS_INFORMATION pi;
	uint32_t exitValue;

	HANDLE hReadPipe = nullptr;
	HANDLE hWritePipe = nullptr;

	std::optional<std::function<void(BuildProcess&, uint32_t)>> endOfProcessCallback;
};