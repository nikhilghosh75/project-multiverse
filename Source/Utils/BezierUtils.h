#pragma once

#include "glm/glm.hpp"

namespace Bezier
{
	glm::vec2 CalculateQuadratic(glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, float t);
};