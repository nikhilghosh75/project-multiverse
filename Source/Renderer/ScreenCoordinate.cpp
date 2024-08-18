#include "ScreenCoordinate.h"
#include "Window.h"

ScreenCoordinate::ScreenCoordinate(glm::vec2 absolutePosition)
	: relativePosition(absolutePosition), anchor(glm::vec2(0, 0))
{
}

ScreenCoordinate::ScreenCoordinate(glm::vec2 relativePosition, Anchor _anchor)
	: relativePosition(relativePosition)
{
	anchor = GetAnchoredPosition(_anchor);
}

ScreenCoordinate::ScreenCoordinate(glm::vec2 _relativePosition, glm::vec2 _relativeAnchor)
{
	relativePosition = _relativePosition;
	anchor = _relativeAnchor;
}

glm::vec2 ScreenCoordinate::GetAbsolutePosition() const
{
	int width, height;
	Window::GetWindowSize(&width, &height);

	return glm::vec2(
		relativePosition.x + (float)width * anchor.x,
		relativePosition.y + (float)height * anchor.y
	);
}

glm::vec2 ScreenCoordinate::GetScreenPosition() const
{
	int width, height;
	Window::GetWindowSize(&width, &height);

	return glm::vec2(
		relativePosition.x / (float)width + anchor.x,
		relativePosition.y / (float)height + anchor.y
	);
}

Rect ScreenCoordinate::CreateRect(ScreenCoordinate position, glm::vec2 dimensions, glm::vec2 pivot)
{
	int width, height;
	Window::GetWindowSize(&width, &height);

	glm::vec2 screenPosition = position.GetScreenPosition();
	glm::vec2 screenDimensions = glm::vec2(dimensions.x / (float)width, dimensions.y / (float)height);

	Rect rect;

	rect.bottom = screenPosition.y + screenDimensions.y * (1 - pivot.y);
	rect.top = screenPosition.y - screenDimensions.y * pivot.y;
	rect.left = screenPosition.x - screenDimensions.x * pivot.x;
	rect.right = screenPosition.x + screenDimensions.x * (1 - pivot.x);

	return rect;
}

glm::vec2 ScreenCoordinate::GetAnchoredPosition(Anchor anchor)
{
	switch (anchor)
	{
	case Anchor::BottomLeft: return glm::vec2(0, 0);
	case Anchor::BottomMiddle: return glm::vec2(0.5f, 0);
	case Anchor::BottomRight: return glm::vec2(1.f, 0);
	case Anchor::MiddleLeft: return glm::vec2(0.f, 0.5f);
	case Anchor::MiddleCenter: return glm::vec2(0.5f, 0.5f);
	case Anchor::MiddleRight: return glm::vec2(1.f, 0.5f);
	case Anchor::TopLeft: return glm::vec2(0.f, 1.f);
	case Anchor::TopCenter: return glm::vec2(0.5f, 1.f);
	case Anchor::TopRight: return glm::vec2(1.f, 1.f);
	}

	return glm::vec2(0, 0);
}
