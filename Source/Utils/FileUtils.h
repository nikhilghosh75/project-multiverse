#pragma once

#include <string>

namespace File
{
	void EnforceForwardSlash(std::string& path);

	void EnforceBackwardSlash(std::string& path);
}