#include "BezierUtils.h"

#include "MathUtils.h"

glm::vec2 Bezier::CalculateQuadratic(glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, float t)
{
	float ax = Math::Lerp(p0.x, p1.x, t);
	float ay = Math::Lerp(p0.y, p1.y, t);

	float bx = Math::Lerp(p1.x, p2.x, t);
	float by = Math::Lerp(p1.y, p2.y, t);

	return { Math::Lerp(ax, bx, t), Math::Lerp(ay, by, t) };
}
