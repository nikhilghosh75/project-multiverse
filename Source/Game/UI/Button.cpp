#include "Button.h"
#include "Input.h"
#include "glm/vec2.hpp"

void Button::Add(Rect rect, std::function<void()> clickedCallback, std::function<void()> hoveredCallback)
{
	glm::vec2 normalizedPosition = Input::GetMouseNormalizedPosition();
	normalizedPosition = normalizedPosition * 2.0f - glm::vec2(1, 1);

	if (rect.bottom > normalizedPosition.y
		&& rect.top < normalizedPosition.y
		&& rect.right > normalizedPosition.x
		&& rect.left < normalizedPosition.x)
	{
		hoveredCallback();

		if (Input::GetMouseButtonDown(MouseButton::Left))
		{
			clickedCallback();
		}
	}
}
