#pragma once

#include "SkeletalSpriteRenderer.h"

#include <string>
#include <string_view>

class UnitySkeletonImporter
{
public:
	static Skeleton Import(const std::string& filepath);

private:
	static glm::vec2 ParsePosition(const std::string_view& positionString);
	static float ParseRotation(const std::string_view& rotationString);
};