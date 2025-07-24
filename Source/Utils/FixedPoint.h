#pragma once

#include <cstdint>

/*
* Exists primarily for TTF file parsing
*/
template <typename T, int DecimalBytes>
class FixedPoint
{
	T rawValue;

	static_assert(DecimalBytes > 0); // There must be a non-zero amount of decimal bytes, otherwise it's an integer
	static_assert(DecimalBytes < sizeof(T) * 8); // The amount of decimal bytes must be less than the overall amount of bytes

public:
	FixedPoint()
		: rawValue(0)
	{
	}

	FixedPoint(T rawValue)
		: rawValue(rawValue)
	{
	}

	FixedPoint(float value)
	{
		rawValue = (T)(value * (1 << DecimalBytes));
	}

	float GetFloat()
	{
		return rawValue / (float)(1 << DecimalBytes);
	}

	T GetRawValue() { return rawValue; }
};

using Fixed2Dot14 = FixedPoint<uint16_t, 14>;