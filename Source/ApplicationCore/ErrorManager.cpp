#include "ErrorManager.h"

#include <iostream>

ErrorManager::ErrorManager()
{
	loggedErrors.reserve(INITIAL_CAPACITY);
}

ErrorManager::~ErrorManager()
{
}

ErrorManager* ErrorManager::Get()
{
	if (instance == nullptr)
	{
		instance = new ErrorManager();
	}

	return instance;
}

void ErrorManager::ReportError(ErrorSeverity severity, std::string functionName, std::string moduleName, uint32_t errorCode, std::string message)
{
	loggedErrors.push_back({ severity, functionName, moduleName, message, errorCode });

	switch (severity)
	{
	case ErrorSeverity::Error:
		std::cout << "ERROR (code " << errorCode << "): " << moduleName << " - " << functionName << ": " << message << std::endl;
		break;
	default:
		break;
	}
}

