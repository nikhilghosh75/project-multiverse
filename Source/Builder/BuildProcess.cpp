#include "BuildProcess.h"

#include <algorithm>
#include <iostream>

BuildProcess::BuildProcess(std::string commandLineArgs)
{
    SECURITY_ATTRIBUTES saAttr{};
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE; // Allow the child to inherit the handle
    saAttr.lpSecurityDescriptor = nullptr;

    CreatePipe(&hReadPipe, &hWritePipe, &saAttr, 0);

    SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0);

	STARTUPINFO si;

	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	si.cb = sizeof(si);
    si.hStdError = hWritePipe;
    si.hStdOutput = hWritePipe;
    si.dwFlags |= STARTF_USESTDHANDLES;

    TCHAR* commandLineParam = new TCHAR[commandLineArgs.size() + 1];
    commandLineParam[commandLineArgs.size()] = 0;
    //As much as we'd love to, we can't use memcpy() because
    //sizeof(TCHAR)==sizeof(char) may not be true:
    std::copy(commandLineArgs.begin(), commandLineArgs.end(), commandLineParam);

    bool success = CreateProcess(NULL,   // No module name (use command line)
        commandLineParam,        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi);

    if (success)
    {

    }
}

BuildProcess::BuildProcess(std::string commandLineArgs, std::function<void(BuildProcess&, uint32_t)> callback)
    : BuildProcess(commandLineArgs)
{
    endOfProcessCallback = callback;
}

BuildProcess::~BuildProcess()
{
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(hWritePipe);
    CloseHandle(hReadPipe);
}

std::string BuildProcess::GetOutput() const
{
    return outputStream.str();
}

void BuildProcess::Update()
{
    char buffer[4096];
    DWORD bytesRead;
    if (ReadFile(hReadPipe, buffer, sizeof(buffer) - 1, &bytesRead, nullptr))
    {
        buffer[bytesRead] = '\0';
        outputStream << buffer;
    }
}

bool BuildProcess::StillRunning()
{
    DWORD exitCode = 0;
    GetExitCodeProcess(pi.hProcess, &exitCode);

    if (exitCode != STILL_ACTIVE)
    {
        exitValue = static_cast<uint32_t>(exitCode);
        std::cout << exitValue << std::endl;
        
        if (endOfProcessCallback)
        {
            (*endOfProcessCallback)(*this, exitValue);
        }
    }

    return exitCode == STILL_ACTIVE;
}
