#pragma once
#include <optional>
#include <utility>

namespace Math
{
	float Clamp(float value, float lower, float upper);

	float CopySign(float x, float y);

	float Lerp(float a, float b, float t);

	float Max(float a, float b);

	float Min(float a, float b);

	std::pair<std::optional<float>, std::optional<float>> QuadraticFormula(float a, float b, float c);
}