#pragma once
#include <array>
#include <stdint.h>
#include "glm/glm.hpp"

enum class InputType
{
	Keyboard,
	Mouse,
	Gamepad,
	Joystick,
	Pen,
	Touchscreen
};

enum class KeyCode
{
	None,
	Backspace,
	Delete,
	Tab,
	Clear,
	Return,
	Pause,
	Escape,
	Space, // 8
	KeypadZero,
	KeypadOne,
	KeypadTwo,
	KeypadThree,
	KeypadFour,
	KeypadFive,
	KeypadSix,
	KeypadSeven, // 16
	KeypadEight,
	KeypadNine,
	Period,
	Divide,
	Multiply,
	Minus,
	Plus,
	KeypadEnter, // 24
	Up,
	Down,
	Left,
	Right,
	Insert,
	Home,
	End,
	PageUp, // 32
	PageDown,
	F1,
	F2,
	F3,
	F4,
	F5,
	F6,
	F7, // 40
	F8,
	F9,
	F10,
	F11,
	F12,
	F13,
	F14,
	F15, // 48
	F16,
	F17,
	F18,
	F19,
	F20,
	AlphaZero,
	AlphaOne,
	AlphaTwo, // 56
	AlphaThree,
	AlphaFour,
	AlphaFive,
	AlphaSix,
	AlphaSeven,
	AlphaEight,
	AlphaNine,
	LeftBracket, // 64
	RightBracket,
	Backslash,
	Underscore,
	Equals,
	Comma,
	Other,
	Slash,
	A, // 72
	B,
	C,
	D,
	E,
	F,
	G,
	H,
	I, // 80
	J,
	K,
	L,
	M,
	N,
	O,
	P,
	Q, // 88
	R,
	S,
	T,
	U,
	V,
	W,
	X,
	Y, // 96
	Z,
	Tilde,
	Numlock,
	Capslock,
	Scrolllock,
	LeftShift,
	RightShift,
	LeftControl, // 104
	RightControl,
	LeftAlt,
	RightAlt,
	LeftCommand,
	RightCommand,
	LeftPlatform,
	RightPlatform,
	AltGr, // 112
	Print,
	Select,
	PrintScreen,
	Help,
	KEY_CODE_END
};

enum class KeyStatus
{
	Pressed,
	PressedThisFrame,
	Released,
	ReleasedThisFrame
};

enum class MouseButton
{
	Left,
	Middle,
	Right
};

enum class MouseStatus
{
	Down,
	DownThisFrame,
	Up,
	UpThisFrame
};

enum class GamepadButton
{
	None,
	DPadNorth,
	DPadSouth,
	DPadEast,
	DPadWest,
	ButtonNorth,
	ButtonSouth,
	ButtonEast,
	ButtonWest,
	LeftShoulder,
	RightShoulder,
	LeftTrigger,
	RightTrigger,
	Start,
	Select,
	LeftStick,
	RightStick
};

class Window;

// A standard input class
// (will likely get refactored later)
class Input
{
	friend class Window;

public:
	static bool GetMouseButton(MouseButton button);
	static bool GetMouseButtonDown(MouseButton button);
	static bool GetMouseButtonUp(MouseButton button);

	static glm::vec2 GetMousePosition();
	static glm::vec2 GetMouseNormalizedPosition();

private:
	static inline std::array<KeyStatus, (size_t)KeyCode::KEY_CODE_END> keyStatuses;
	static inline MouseStatus leftMouseStatus;
	static inline MouseStatus rightMouseStatus;
	static inline MouseStatus middleMouseStatus;
	static inline glm::vec2 mousePosition;

	static void Initialize();
	static void Update();

	static void ChangeKeyState(KeyCode keycode, bool justPressed);
	static void ChangeMouseState(MouseButton button, bool justPressed);
	static void ChangeMousePosition(float x, float y);
};
