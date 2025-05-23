#pragma once

#include "SkeletalSpriteRenderer.h"

#include <string>
#include <string_view>

class UnitySkeletonImporter
{
public:
	static Skeleton Import(const std::string& filepath);

};