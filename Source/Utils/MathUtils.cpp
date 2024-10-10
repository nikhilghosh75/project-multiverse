#include "MathUtils.h"

float Math::Clamp(float value, float lower, float upper)
{
	return Max(lower, Min(value, upper));
}

float Math::Lerp(float a, float b, float t)
{
	return ((b - a) * t) + a;;
}

float Math::Max(float a, float b)
{
	return a > b ? a : b;
}

float Math::Min(float a, float b)
{
	return a < b ? a : b;
}
