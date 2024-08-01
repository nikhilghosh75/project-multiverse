#pragma once
#include "glm/glm.hpp"
#include "Rect.h"

enum Anchor
{
	TopLeft,
	TopCenter,
	TopRight,
	MiddleLeft,
	MiddleCenter,
	MiddleRight,
	BottomLeft,
	BottomMiddle,
	BottomRight
};

class ScreenCoordinate
{
public:
	ScreenCoordinate(glm::vec2 absolutePosition);
	ScreenCoordinate(glm::vec2 relativePosition, Anchor anchor);
	ScreenCoordinate(glm::vec2 relativePosition, glm::vec2 relativeAnchor);

	glm::vec2 relativePosition;
	glm::vec2 anchor;

	glm::vec2 GetAbsolutePosition() const;
	glm::vec2 GetScreenPosition() const;

	static Rect CreateRect(ScreenCoordinate position, glm::vec2 dimensions, glm::vec2 pivot);

private:
	static glm::vec2 GetAnchoredPosition(Anchor anchor);


};