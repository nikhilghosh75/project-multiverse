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

	Window();

	~Window();

	void Process();

	static void GetWindowSize(int* width, int* height);

	static void ChangeKeyState(KeyCode keycode, bool justPressed);
	static void ChangeMouseState(MouseButton button, bool justPressed);

	static void OnResize(unsigned int width, unsigned int height);

	static KeyCode SystemParamToKeycode(unsigned int param);
private:
	static inline Window* window;

	Device* device;
};

LRESULT CALLBACK WndProc(HWND window, int wm, WPARAM wParam, LPARAM lParam);