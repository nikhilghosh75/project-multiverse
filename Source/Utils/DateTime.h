#pragma once
#include <cstdint>
#include <string>

class DateTime
{
public:
	DateTime();
	DateTime(uint64_t _ticks);
	
	std::string Str() const;

	static DateTime Now();



private:
	uint64_t ticks; // Microseconds since January 1st, 1970
};