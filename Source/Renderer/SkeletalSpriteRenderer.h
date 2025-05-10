#pragma once

#include "glm/mat4x4.hpp"

#include <array>
#include <vector>

class Bone
{
public:
	glm::vec2 localPosition;
	float localRotation;
	std::vector<Bone*> children;
	Bone* parent;
	float length;

	glm::mat4 GetLocalTransform() const;

	glm::vec2 GetAbsolutePosition() const;
	float GetAbsoluteRotation() const;
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
