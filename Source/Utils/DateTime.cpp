#include "DateTime.h"

#include "AssertUtils.h"

#include <ctime>
#include <sstream>

#include <Windows.h>

const int DAYSTOMONTH[] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };

const uint64_t MICROSECONDS_IN_SECOND = 1000000;

// The amount of seconds between January 1st, 1601 (the Windows epoch) and 1970 (the Unix epoch)
const uint64_t SECONDS_SINCE_1601 = 11644473600;
const uint64_t MICROSECONDS_SINCE_1601 = SECONDS_SINCE_1601 * MICROSECONDS_IN_SECOND;

DateTime::DateTime()
	: ticks(0)
{
}

DateTime::DateTime(uint64_t _ticks)
	: ticks(_ticks)
{
}

std::string DateTime::Str() const
{
	std::stringstream ss;

	std::time_t secondsSinceEpoch = ticks / MICROSECONDS_IN_SECOND;
	std::tm* timeInfo = std::localtime(&secondsSinceEpoch);
	
	ss << timeInfo->tm_mon + 1 << "/" << timeInfo->tm_mday << "/" << timeInfo->tm_year + 1900
		<< " " << timeInfo->tm_hour << ":" << timeInfo->tm_min << ":" << timeInfo->tm_sec;

	return ss.str();
}

DateTime DateTime::Now()
{
	FILETIME time;
	GetSystemTimeAsFileTime(&time);

	ULARGE_INTEGER timeInt;
	timeInt.HighPart = time.dwHighDateTime;
	timeInt.LowPart = time.dwLowDateTime;

	uint64_t microsecondsSinceEpoch = (static_cast<uint64_t>(timeInt.QuadPart) / 10);
	ASSERT(microsecondsSinceEpoch > MICROSECONDS_SINCE_1601);

	return DateTime(microsecondsSinceEpoch - MICROSECONDS_SINCE_1601);
}
