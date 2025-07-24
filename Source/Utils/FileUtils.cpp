#include "FileUtils.h"

void File::EnforceForwardSlash(std::string& path)
{
	for (int i = 0; i < path.size(); i++)
	{
		if (path[i] == '\\')
		{
			path[i] = '/';
		}
	}
}

void File::EnforceBackwardSlash(std::string& path)
{
	for (int i = 0; i < path.size(); i++)
	{
		if (path[i] == '/')
		{
			path[i] = '\\';
		}
	}
}

bool File::HasExtension(const std::string& path, const std::string& ext)
{
	if (ext.size() > path.size())
	{
		return false;
	}

	for (int i = 1; i <= ext.size(); i++)
	{
		if (path[path.size() - i] != ext[ext.size() - i])
		{
			return false;
		}
	}

	return path[path.size() - ext.size() - 1] == '.';
}
