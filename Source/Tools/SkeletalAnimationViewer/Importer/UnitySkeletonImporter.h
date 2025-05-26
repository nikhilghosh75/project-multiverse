#pragma once

#include "SkeletalSpriteRenderer.h"

#include <string>
#include <string_view>
#include <utility>

class SkeletonDebugInfo;

class UnitySkeletonImporter
{
public:
	static std::pair<Skeleton, SkeletonDebugInfo> Import(const std::string& filepath);

};