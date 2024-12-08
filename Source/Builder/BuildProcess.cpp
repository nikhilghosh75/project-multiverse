#include "BuildProcess.h"

#include <algorithm>

BuildProcess::BuildProcess(std::string commandLineArgs)
{
	STARTUPINFO si;

	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	si.cb = sizeof(si);

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

BuildProcess::~BuildProcess()
{
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

bool BuildProcess::StillRunning()
{
    DWORD exitCode = 0;
    GetExitCodeProcess(pi.hProcess, &exitCode);

    if (exitCode != STILL_ACTIVE)
    {
        exitValue = static_cast<uint32_t>(exitCode);
        // TODO: Add Callback
    }

    return exitCode == STILL_ACTIVE;
}
