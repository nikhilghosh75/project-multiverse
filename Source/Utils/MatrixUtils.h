#pragma once

#include "glm/glm.hpp"

namespace Matrix
{
	glm::mat3 TRS2D(glm::vec2 translation, float rotation, glm::vec2 scale);
}