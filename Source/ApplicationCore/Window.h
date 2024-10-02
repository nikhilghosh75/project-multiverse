#pragma once
#include <windows.h>

class Device;

enum class KeyCode;
enum class MouseButton;

// A class representing a window
class Window
{
public:
	static bool windowRunning;

	static bool imguiEnabled;

	Window();

	~Window();

	void Process();

	static void GetWindowSize(int* width, int* height);

	static void ChangeKeyState(KeyCode keycode, bool justPressed);
	static void ChangeMouseState(MouseButton button, bool justPressed);
	static void ChangeMousePosition(int x, int y);

	static void OnResize(unsigned int width, unsigned int height);

	static KeyCode SystemParamToKeycode(unsigned int param);

	static HWND GetWindowHandle();

private:
	static inline Window* window;

	Device* device;
};

LRESULT CALLBACK WndProc(HWND window, int wm, WPARAM wParam, LPARAM lParam);