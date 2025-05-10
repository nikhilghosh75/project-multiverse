#pragma once

#include "glm/glm.hpp"

class Quat
{
public:
	Quat();
	Quat(float x, float y, float z, float w);
	
	glm::vec3 ToEulerAngles() const;

	float x, y, z, w;
};