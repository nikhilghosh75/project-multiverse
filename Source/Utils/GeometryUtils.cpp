#include "GeometryUtils.h"

bool Geometry::DoesCornerBendOutwards(glm::vec2 previousVertex, glm::vec2 currentVertex, const glm::vec2& nextVertex)
{
	return glm::cross(glm::vec3(nextVertex - currentVertex, 0), glm::vec3(previousVertex - currentVertex, 0)).z < 0;
}

bool Geometry::IsPointInTriangle(const glm::vec2& point, const glm::vec2& a, const glm::vec2& b, const glm::vec2& c)
{
    float area = 0.5f * (-b.y * c.x + a.y * (-b.x + c.x) + a.x * (b.y - c.y) + b.x * c.y);
    float s = 1 / (2 * area) * (a.y * c.x - a.x * c.y + (c.y - a.y) * point.x + (a.x - c.x) * point.y);
    float t = 1 / (2 * area) * (a.x * b.y - a.y * b.x + (a.y - b.y) * point.x + (b.x - a.x) * point.y);
    float u = 1 - s - t;
    return s > 0 && t > 0 && u > 0;
}
