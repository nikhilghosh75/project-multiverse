#include "Window.h"
#include "Device.h"
#include "Input.h"
#include "backends/imgui_impl_win32.h"
#include <windowsx.h>

HINSTANCE g_window_instance;
HWND g_window;

bool Window::windowRunning = false;
bool Window::imguiEnabled = false;

const KeyCode SYSTEM_TO_KEYCODE[] =
{
	KeyCode::None, KeyCode::None, KeyCode::None, KeyCode::None,							// 0x04
	KeyCode::None, KeyCode::None, KeyCode::None, KeyCode::None,							// 0x08
	KeyCode::Backspace, KeyCode::Tab, KeyCode::None, KeyCode::None,						// 0x0C
	KeyCode::Clear, KeyCode::Return, KeyCode::None, KeyCode::None,						// 0x10
	KeyCode::LeftShift, KeyCode::LeftControl, KeyCode::LeftAlt, KeyCode::Pause,			// 0x14
	KeyCode::Capslock, KeyCode::None, KeyCode::None, KeyCode::None,						// 0x18
	KeyCode::None, KeyCode::None, KeyCode::None, KeyCode::Escape,						// 0x1C
	KeyCode::None, KeyCode::None, KeyCode::None, KeyCode::None,							// 0x20
	KeyCode::Space, KeyCode::PageUp, KeyCode::PageDown, KeyCode::End,					// 0x24
	KeyCode::Home, KeyCode::Left, KeyCode::Up, KeyCode::Right, 							// 0x28
	KeyCode::Down,KeyCode::Select, KeyCode::Print, KeyCode::None,						// 0x2C
	KeyCode::PrintScreen, KeyCode::Insert, KeyCode::Delete, KeyCode::Help,				// 0x30
	KeyCode::AlphaZero, KeyCode::AlphaOne, KeyCode::AlphaTwo, KeyCode::AlphaThree,		// 0x34
	KeyCode::AlphaFour, KeyCode::AlphaFive, KeyCode::AlphaSix, KeyCode::AlphaSeven,		// 0x38
	KeyCode::AlphaEight, KeyCode::AlphaNine, KeyCode::None, KeyCode::None,				// 0x3C
	KeyCode::None, KeyCode::None, KeyCode::None, KeyCode::None,							// 0x40
	KeyCode::A, KeyCode::B, KeyCode::C, KeyCode::D,										// 0x44
	KeyCode::E, KeyCode::F, KeyCode::G, KeyCode::H,										// 0x48
	KeyCode::I, KeyCode::J, KeyCode::K, KeyCode::L,										// 0x4C
	KeyCode::M, KeyCode::N, KeyCode::O, KeyCode::P,										// 0x50
	KeyCode::Q, KeyCode::R, KeyCode::S, KeyCode::T,										// 0x54
	KeyCode::U, KeyCode::V, KeyCode::W, KeyCode::X,										// 0x58
	KeyCode::Y, KeyCode::Z, KeyCode::None, KeyCode::None,								// 0x5C
	KeyCode::None, KeyCode::None, KeyCode::None, KeyCode::None,							// 0x60
	KeyCode::KeypadZero, KeyCode::KeypadOne, KeyCode::KeypadTwo, KeyCode::KeypadThree,	// 0x64
	KeyCode::KeypadFour, KeyCode::KeypadFive, KeyCode::KeypadSix, KeyCode::KeypadSeven,	// 0x68
	KeyCode::KeypadEight, KeyCode::KeypadNine, KeyCode::Multiply, KeyCode::Plus,		// 0x6C
	KeyCode::None, KeyCode::None, KeyCode::Period, KeyCode::Divide,						// 0x70
	KeyCode::F1, KeyCode::F2, KeyCode::F3, KeyCode::F4,									// 0x74
	KeyCode::F5, KeyCode::F6, KeyCode::F7, KeyCode::F8,									// 0x78
};

Window::Window()
{
	WNDCLASSEX wc{};

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.lpfnWndProc = (WNDPROC)WndProc;
	wc.hInstance = GetModuleHandle(nullptr);
	wc.lpszClassName = L"Project Multiverse";
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.style = CS_OWNDC;

	if (!RegisterClassEx(&wc))
	{
		// TODO: Output the following error code
		// "Win32: Class cannot be registered"
		exit(0);
	}

	HWND hwnd = CreateWindowEx(
		0,
		L"Project Multiverse",
		L"Project Multiverse",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, g_window_instance, NULL
	);

	if (hwnd == NULL)
	{
		// TODO: Output the following error code
		// "Win32: Window cannot be created"
		exit(0);
	}

	g_window = hwnd;

	ShowWindow(hwnd, SW_SHOWDEFAULT);
	UpdateWindow(hwnd);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	windowRunning = true;
	window = this;

	device = new Device();
	device->ConnectWin32(hwnd, g_window_instance);

	Input::Initialize();
}

Window::~Window()
{
	delete device;

	DestroyWindow(g_window);
	PostQuitMessage(0);
}

void Window::Process()
{
	Input::Update();

	MSG message;

	while (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE) > 0)
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
	}
}

void Window::GetWindowSize(int* width, int* height)
{
	int titlebarHeight = GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CXPADDEDBORDER);
	RECT rect;
	GetWindowRect(g_window, &rect);

	// TODO: Figure out why there are 14 extra pixels
	*width = rect.right - rect.left;
	*height = rect.bottom - rect.top - titlebarHeight - 14;
}

void Window::ChangeKeyState(KeyCode keycode, bool justPressed)
{
	Input::ChangeKeyState(keycode, justPressed);
}

void Window::ChangeMouseState(MouseButton button, bool justPressed)
{
	Input::ChangeMouseState(button, justPressed);
}

void Window::ChangeMousePosition(int x, int y)
{
	Input::ChangeMousePosition(x, y);
}

KeyCode Window::SystemParamToKeycode(unsigned int param)
{
	return KeyCode();
}

HWND Window::GetWindowHandle()
{
	return g_window;
}

void Window::OnResize(unsigned int width, unsigned int height)
{
	if (window == nullptr)
	{
		return;
	}

	window->device->shouldResizeFramebuffer = true;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WndProc(HWND window, int wm, WPARAM wParam, LPARAM lParam)
{
	if (Window::imguiEnabled)
	{
		if (ImGui_ImplWin32_WndProcHandler(window, wm, wParam, lParam))
			return true;
	}

	switch (wm)
	{
	case WM_DESTROY:
		Window::windowRunning = false;
		break;
	case WM_PAINT:
		ValidateRect(window, NULL);
		break;
	case WM_LBUTTONDOWN:
		Window::ChangeMouseState(MouseButton::Left, true);
		break;
	case WM_LBUTTONUP:
		Window::ChangeMouseState(MouseButton::Left, false);
		break;
	case WM_MBUTTONDOWN:
		Window::ChangeMouseState(MouseButton::Middle, true);
		break;
	case WM_MBUTTONUP:
		Window::ChangeMouseState(MouseButton::Middle, false);
		break;
	case WM_RBUTTONDOWN:
		Window::ChangeMouseState(MouseButton::Right, true);
		break;
	case WM_RBUTTONUP:
		Window::ChangeMouseState(MouseButton::Right, false);
		break;
	case WM_SIZE:
	{
		unsigned int width = LOWORD(lParam);
		unsigned int height = HIWORD(lParam);
		Window::OnResize(width, height);
		break;
	}
	default:
		return DefWindowProc(window, wm, wParam, lParam);
		break;
	}
}
