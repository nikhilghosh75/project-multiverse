#pragma once

#include "Texture.h"

#include "Rect.h"

#include "glm/mat4x4.hpp"

#include <array>
#include <vector>

class Bone
{
public:
	glm::vec2 localPosition;
	float localRotation;
	glm::mat4 inverseBindPose;

	std::vector<Bone*> children;
	Bone* parent;
	float length;

	glm::mat4 GetLocalTransform() const;
	glm::mat4 GetAbsoluteTransform() const;

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
	glm::vec2 skinnedPosition;
	std::array<BoneWeight, 4> weights;
};

class Skeleton
{
public:
	Skeleton();

	std::vector<Bone> bones;
};

class SkeletalSprite
{
public:
	SkeletalSprite();

	struct Layer
	{
		std::string name;
		std::vector<SpriteVertex> vertices;
		std::vector<int> indices;

		Rect uvRect;

		// Relative to ?
		glm::vec2 center;
		float width;
		float height;
	};

	void SkinVertices();

	std::vector<Layer> layers;
	Skeleton skeleton;
	Texture* texture;
};