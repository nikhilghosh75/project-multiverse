#pragma once
#include <cstdint>
#include <string>

/*
* Represents the time
*/
class DateTime
{
public:
	DateTime();
	DateTime(uint64_t _ticks);
	
	std::string Str() const;

	static DateTime Now();

	uint64_t GetTicks() { return ticks; }

private:
	uint64_t ticks; // Microseconds since January 1st, 1970
};