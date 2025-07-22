#include "StringUtils.h"

void String::FixNegativeChars(std::string& s)
{
	for (int i = 0; i < s.size(); i++)
	{
		if (s[i] < 0)
		{
			s[i] += 128;
		}
	}
}

void String::ReplaceAll(std::string& s, char from, char to)
{
	for (int i = 0; i < s.size(); i++)
	{
		if (s[i] == from)
		{
			s[i] = to;
		}
	}
}

void String::ReplaceAll(std::string& s, const std::string& from, const std::string& to)
{
	// Taken from https://stackoverflow.com/questions/3418231/replace-part-of-a-string-with-another-string
	if (from.empty())
	{
		return;
	}

	size_t start_pos = 0;
	while ((start_pos = s.find(from, start_pos)) != std::string::npos) 
	{
		s.replace(start_pos, from.length(), to);
		start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
}
