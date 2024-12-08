#pragma once

#include <string>

class Registry
{
public:
	static std::string QueryValue(const std::string& subkey, const std::string& value);
};