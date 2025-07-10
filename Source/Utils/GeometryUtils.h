#pragma once

#include "glm/glm.hpp"

namespace Geometry
{
	bool DoesCornerBendOutwards(glm::vec2 previousVertex, glm::vec2 currentVertex, const glm::vec2& nextVertex);

	bool IsPointInTriangle(const glm::vec2& point, const glm::vec2& a, const glm::vec2& b, const glm::vec2& c);
}