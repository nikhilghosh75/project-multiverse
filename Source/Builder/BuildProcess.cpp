#include "BuildProcess.h"

#include "BuildSystem.h"

#include <algorithm>
#include <iostream>

static int asyncPipeCount;

void CreateAsyncPipe(HANDLE* outRead, HANDLE* outWrite)
{
    wchar_t pipeName[64] = { L'\0' };
    swprintf(pipeName, L"\\\\.\\pipe\\buildprocess%i", asyncPipeCount);

    asyncPipeCount++;

    *outRead = CreateNamedPipeW(
        pipeName,
        PIPE_ACCESS_INBOUND | FILE_FLAG_OVERLAPPED,
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
        2,
        4096,
        0,
        0,
        nullptr
    );

    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;
    *outWrite = CreateFileW(pipeName, GENERIC_WRITE, 0, &saAttr, OPEN_EXISTING, 0, 0);
}

BuildProcess::BuildProcess(std::string commandLineArgs)
    : exitValue(0)
{
    CreateAsyncPipe(&hReadPipe, &hWritePipe);
    CreateAsyncPipe(&hReadErrorPipe, &hWriteErrorPipe);

    SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0);

	STARTUPINFO si;

	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	si.cb = sizeof(si);
    si.hStdError = hWriteErrorPipe;
    si.hStdOutput = hWritePipe;
    si.dwFlags = STARTF_USESTDHANDLES;

    TCHAR* commandLineParam = new TCHAR[commandLineArgs.size() + 1];
    commandLineParam[commandLineArgs.size()] = 0;
    //As much as we'd love to, we can't use memcpy() because
    //sizeof(TCHAR)==sizeof(char) may not be true:
    std::copy(commandLineArgs.begin(), commandLineArgs.end(), commandLineParam);

    bool success = CreateProcess(NULL,   // No module name (use command line)
        commandLineParam,        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        TRUE,          // Set handle inheritance to TRUE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi);

    COMMTIMEOUTS timeouts;
    timeouts.ReadIntervalTimeout = 0;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.ReadTotalTimeoutConstant = 3;
    timeouts.WriteTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = 0;

    SetCommTimeouts(hReadPipe, &timeouts);

    BuildSystem::Get()->output << "Calling command line for node " << nodeId << ": " << commandLineArgs << "\n";

    if (!success)
    {
        startupSucceeded = false;
        BuildSystem::Get()->output << "Command failed" << std::endl;
    }
}

BuildProcess::BuildProcess(std::string commandLineArgs, uint32_t _nodeId)
    : BuildProcess(commandLineArgs)
{
    nodeId = _nodeId;
}

BuildProcess::BuildProcess(std::string commandLineArgs, std::function<void(BuildProcess&, uint32_t)> callback)
    : BuildProcess(commandLineArgs)
{
    endOfProcessCallback = callback;
}

BuildProcess::BuildProcess(std::string commandLineArgs, std::function<void(BuildProcess&, uint32_t)> callback, uint32_t _nodeId)
    : BuildProcess(commandLineArgs)
{
    endOfProcessCallback = callback;
    nodeId = _nodeId;
}

BuildProcess::~BuildProcess()
{
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(hWritePipe);
    CloseHandle(hReadPipe);
    CloseHandle(hWriteErrorPipe);
    CloseHandle(hReadErrorPipe);
}

std::string BuildProcess::GetOutput() const
{
    return outputStream.str();
}

void BuildProcess::Terminate()
{
    if (startupSucceeded)
    {
        TerminateProcess(pi.hProcess, 137);
    }
}

void BuildProcess::Update()
{
    StillRunning();

    DWORD bytesAvailable = 0;
    if (!PeekNamedPipe(hReadPipe, nullptr, 0, nullptr, &bytesAvailable, nullptr)) 
    {
        return;
    }

    if (bytesAvailable > 0) 
    {
        char buffer[4096];
        DWORD bytesRead = 0;
        if (ReadFile(hReadPipe, buffer, bytesAvailable, &bytesRead, nullptr)) 
        {
            buffer[bytesRead] = '\0';
            outputStream << buffer; 

            BuildSystem::Get()->LogOutputFromFile(buffer, nodeId);
        }
    }
}

bool BuildProcess::StillRunning()
{
    if (!startupSucceeded)
    {
        return false;
    }

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

bool BuildProcess::WasStartupSuccessful()
{
    return startupSucceeded;
}
