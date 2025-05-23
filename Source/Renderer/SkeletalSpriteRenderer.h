#pragma once

#include "Texture.h"

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

struct SpriteVertex
{
	glm::vec2 position;
	glm::vec2 uv;
	std::array<BoneWeight, 4> weights;
};

class Skeleton
{
public:
	Skeleton();

	std::vector<Bone> bones;
	std::vector<SpriteVertex> vertices;
};

class SkeletalSprite
{
public:
	struct Layer
	{
		std::string name;
		Texture* texture;
		std::vector<SpriteVertex> vertices;
	};
};