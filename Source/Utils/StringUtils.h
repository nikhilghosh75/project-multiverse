#pragma once

#include <string>

namespace String
{
	void FixNegativeChars(std::string& s);

	void ReplaceAll(std::string& s, char from, char to);
	void ReplaceAll(std::string& s, const std::string& from, const std::string& to);
}