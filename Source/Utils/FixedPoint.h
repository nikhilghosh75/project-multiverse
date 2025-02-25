#pragma once

#include <cstdint>

template <typename T, int DecimalBytes>
class FixedPoint
{
	T rawValue;

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