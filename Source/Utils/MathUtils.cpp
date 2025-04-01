#include "MathUtils.h"
#include "math.h"

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

std::pair<std::optional<float>, std::optional<float>> Math::QuadraticFormula(float a, float b, float c)
{
	std::pair<std::optional<float>, std::optional<float>> result;

	if (a == 0)
	{
		if (b != 0)
		{
			result.first = -c / b;
		}
	}
	else
	{
		float discriminant = b * b - 4 * a * c;
		if (discriminant >= 0)
		{
			float s = sqrtf(discriminant);
			result.first = (-b + s) / (2 * a);
			result.second = (-b - s) / (2 * a);
		}
	}

	return result;
}
