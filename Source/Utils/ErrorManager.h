#pragma once

#include <cstdint>
#include <string>
#include <vector>

enum class ErrorSeverity
{
	Minimal,
	Warning,
	Error,
	Severe
};


struct Error
{
	ErrorSeverity severity;
	std::string functionName;
	std::string moduleName;
	std::string message;
	uint32_t errorCode;
};

class ErrorManager
{
public:
	ErrorManager();
	~ErrorManager();

	static ErrorManager* Get();

	void ReportError(ErrorSeverity severity, std::string functionName, std::string moduleName, uint32_t errorCode, std::string message = "");

private:
	static const int INITIAL_CAPACITY = 16;

	static inline ErrorManager* instance;

	std::vector<Error> loggedErrors;
};