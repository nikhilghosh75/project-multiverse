#pragma once

#include "glm/mat4x4.hpp"

#include <array>
#include <vector>

class Bone
{
public:
	glm::mat4 localTransform;
	std::vector<Bone*> children;

};

struct BoneWeight
{
	int boneIndex;
	float boneWeight;
};

class Skeleton
{
public:
	Skeleton();

	std::vector<Bone> bones;
};

class SkinnedVertex
{
public:
	SkinnedVertex();

	glm::vec2 position;
	glm::vec2 uv;
	std::array<BoneWeight, 4> boneWeights;
};