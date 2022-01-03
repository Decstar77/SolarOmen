#include "platform/SolarPlatform.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Xinput.h>

#if SOLAR_PLATFORM_WINDOWS
namespace sol
{
	struct Win32State
	{
		HWND window;
		HINSTANCE hinstance;
	};

	static Win32State winState = {};


	bool8 Platform::Intialize(int32 x, int32 y, int32 width, int32 height)
	{
		//winState.hinstance = GetModuleHandleA(0);

		//RECT client_rect;
		//client_rect.left = 0;
		//client_rect.right = (LONG)width;
		//client_rect.top = 0;
		//client_rect.bottom = (LONG)height;

		//DWORD window_style = WS_VISIBLE | (WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME);
		//AdjustWindowRect(&client_rect, window_style, FALSE);

		//WNDCLASSEXA window_class = {};
		//window_class.cbSize = sizeof(window_class);
		//window_class.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
		//window_class.lpfnWndProc = &WindProc;
		//window_class.cbClsExtra = 0;
		//window_class.cbWndExtra = 0;
		//window_class.hInstance = 0;
		//window_class.hIcon = nullptr;
		//window_class.hIconSm = nullptr;
		//window_class.hCursor = nullptr;
		//window_class.hbrBackground = nullptr;
		//window_class.lpszClassName = "GaMe eNgInE";

		//RegisterClassExA(&window_class);

		//HWND window = CreateWindowExA(
		//	NULL, window_class.lpszClassName, "GaMe eNgInE", window_style,
		//	CW_USEDEFAULT, CW_USEDEFAULT, client_rect.right - client_rect.left, client_rect.bottom - client_rect.top,
		//	NULL, NULL, NULL, NULL);

		//GetClientRect(window, &client_rect);

		//ps->window = (int64)window;
		//ps->clientWidth = client_rect.right - client_rect.left;
		//ps->clientHeight = client_rect.bottom - client_rect.top;
		//ps->aspect = (real32)ps->clientWidth / (real32)ps->clientHeight;

		//if (window)
		//{
		//	// NOTE: Adjust window pos ect
		//	int32 window_pos_x = GetSystemMetrics(SM_CXSCREEN) / 2 - ps->clientWidth / 2;
		//	int32 window_pos_y = GetSystemMetrics(SM_CYSCREEN) / 2 - ps->clientHeight / 2;
		//	SetWindowPos(window, NULL, window_pos_x, window_pos_y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		//	//================================================================//

		//	// NOTE: Initialize raw input --- duh
		//	InitializeRawInput(ps);
		//	//================================================================//

		//	platformRunning = true;

		//	PlatformState::Initialize(ps);

		//	return true;
		//}

		return false;
	}

	void Platform::Shutdown()
	{

	}
}
#endif