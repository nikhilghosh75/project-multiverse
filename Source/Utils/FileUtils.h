#pragma once

#include <string>

enum FileFlags
{
	LittleEndian = 1,
	BigEndian = 2,
};

inline FileFlags operator|(FileFlags a, FileFlags b)
{
	return static_cast<FileFlags>(static_cast<int>(a) | static_cast<int>(b));
}

inline FileFlags operator&(FileFlags a, FileFlags b)
{
	return static_cast<FileFlags>(static_cast<int>(a) & static_cast<int>(b));
}

namespace File
{
	void EnforceForwardSlash(std::string& path);

	void EnforceBackwardSlash(std::string& path);

	bool HasExtension(const std::string& path, const std::string& ext);
}