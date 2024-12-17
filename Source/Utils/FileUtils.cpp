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
