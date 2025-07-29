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
	BuildProcess(std::string commandLineArgs, uint32_t nodeId);
	BuildProcess(std::string commandLineArgs, std::function<void(BuildProcess&, uint32_t)>);
	BuildProcess(std::string commandLineArgs, std::function<void(BuildProcess&, uint32_t)>, uint32_t nodeId);
	~BuildProcess();

	std::string GetOutput() const;

	void Terminate();

	void Update();

	bool StillRunning();
	bool WasStartupSuccessful();

private:
	std::stringstream outputStream;

	bool startupSucceeded = true;
	int nodeId;

	PROCESS_INFORMATION pi;
	uint32_t exitValue;

	HANDLE hReadPipe = nullptr;
	HANDLE hWritePipe = nullptr;
	HANDLE hReadErrorPipe = nullptr;
	HANDLE hWriteErrorPipe = nullptr;

	std::optional<std::function<void(BuildProcess&, uint32_t)>> endOfProcessCallback;
};