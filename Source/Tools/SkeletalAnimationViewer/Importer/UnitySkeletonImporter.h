#pragma once

#include "SkeletalSpriteRenderer.h"

#include <string>
#include <string_view>
#include <utility>

class SkeletonDebugInfo;

class UnitySkeletonImporter
{
public:
	static void Import(const std::string& filepath, Skeleton& skeleton, SkeletonDebugInfo& debugInfo);

};