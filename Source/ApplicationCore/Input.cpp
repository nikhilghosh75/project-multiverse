#include "Input.h"
#include "Window.h"

bool Input::GetMouseButton(MouseButton button)
{
	switch (button)
	{
	case MouseButton::Left: return leftMouseStatus == MouseStatus::Down
		|| leftMouseStatus == MouseStatus::DownThisFrame;
	case MouseButton::Middle: return middleMouseStatus == MouseStatus::Down
		|| middleMouseStatus == MouseStatus::DownThisFrame;
	case MouseButton::Right: return rightMouseStatus == MouseStatus::Down
		|| rightMouseStatus == MouseStatus::DownThisFrame;
	}

	return false;
}

bool Input::GetMouseButtonDown(MouseButton button)
{
	switch (button)
	{
	case MouseButton::Left: return leftMouseStatus == MouseStatus::DownThisFrame;
	case MouseButton::Middle: return middleMouseStatus == MouseStatus::DownThisFrame;
	case MouseButton::Right: return rightMouseStatus == MouseStatus::DownThisFrame;
	}

	return false;
}

bool Input::GetMouseButtonUp(MouseButton button)
{
	switch (button)
	{
	case MouseButton::Left: return leftMouseStatus == MouseStatus::UpThisFrame;
	case MouseButton::Middle: return middleMouseStatus == MouseStatus::UpThisFrame;
	case MouseButton::Right: return rightMouseStatus == MouseStatus::UpThisFrame;
	}

	return false;
}

glm::vec2 Input::GetMousePosition()
{
	POINT point;
	GetCursorPos(&point);

	ScreenToClient(Window::GetWindowHandle(), &point);

	return glm::vec2(point.x, point.y);
}

glm::vec2 Input::GetMouseNormalizedPosition()
{
	int width;
	int height;
	Window::GetWindowSize(&width, &height);

	return glm::vec2(GetMousePosition().x / width, GetMousePosition().y / height);
}

void Input::Initialize()
{
	for (int i = 0; i < (int)KeyCode::KEY_CODE_END; i++)
	{
		keyStatuses[i] = KeyStatus::Released;
	}

	leftMouseStatus = MouseStatus::Up;
	middleMouseStatus = MouseStatus::Up;
	rightMouseStatus = MouseStatus::Up;
}

void Input::Update()
{
	for (int i = 0; i < (int)KeyCode::KEY_CODE_END; i++)
	{
		if (keyStatuses[i] == KeyStatus::PressedThisFrame)
		{
			keyStatuses[i] = KeyStatus::Pressed;
		}
		else if (keyStatuses[i] == KeyStatus::ReleasedThisFrame)
		{
			keyStatuses[i] = KeyStatus::Released;
		}
	}

	if (leftMouseStatus == MouseStatus::DownThisFrame)
	{
		leftMouseStatus = MouseStatus::Down;
	}
	else if (leftMouseStatus == MouseStatus::UpThisFrame)
	{
		leftMouseStatus = MouseStatus::Up;
	}

	if (rightMouseStatus == MouseStatus::DownThisFrame)
	{
		rightMouseStatus = MouseStatus::Down;
	}
	else if (rightMouseStatus == MouseStatus::UpThisFrame)
	{
		rightMouseStatus = MouseStatus::Up;
	}

	if (middleMouseStatus == MouseStatus::DownThisFrame)
	{
		middleMouseStatus = MouseStatus::Down;
	}
	else if (middleMouseStatus == MouseStatus::UpThisFrame)
	{
		middleMouseStatus = MouseStatus::Up;
	}
}

void Input::ChangeKeyState(KeyCode keycode, bool justPressed)
{
	if (justPressed)
	{
		keyStatuses[(size_t)keycode] = KeyStatus::PressedThisFrame;
	}
	else
	{
		keyStatuses[(size_t)keycode] = KeyStatus::ReleasedThisFrame;
	}
}

void Input::ChangeMouseState(MouseButton button, bool justPressed)
{
	if (justPressed)
	{
		switch (button)
		{
		case MouseButton::Left:
			leftMouseStatus = MouseStatus::DownThisFrame;
			break;
		case MouseButton::Middle:
			middleMouseStatus = MouseStatus::DownThisFrame;
			break;
		case MouseButton::Right:
			rightMouseStatus = MouseStatus::DownThisFrame;
			break;
		default:
			break;
		}
	}
	else
	{
		switch (button)
		{
		case MouseButton::Left:
			leftMouseStatus = MouseStatus::UpThisFrame;
			break;
		case MouseButton::Middle:
			middleMouseStatus = MouseStatus::UpThisFrame;
			break;
		case MouseButton::Right:
			rightMouseStatus = MouseStatus::UpThisFrame;
			break;
		default:
			break;
		}
	}
}

void Input::ChangeMousePosition(float x, float y)
{
	mousePosition = glm::vec2(x, y);
}
