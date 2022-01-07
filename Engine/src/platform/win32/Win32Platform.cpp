#include "platform/SolarPlatform.h"

#include "core/SolarEvent.h"
#include "Win32State.h"
#include <core/SolarLogging.h>
#include <core/SolarInput.h>
#include "core/SolarMath.h"

#if SOLAR_PLATFORM_WINDOWS
namespace sol
{
	static Win32State winState = {};

	PlatformFile Platform::LoadEntireFile(const String& path, bool32 metaDataOnly)
	{
		PlatformFile result = {};
		result.path = path;

		HANDLE FileHandle = CreateFileA(path.GetCStr(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
		if (FileHandle != INVALID_HANDLE_VALUE)
		{
			LARGE_INTEGER FileSize;
			if (GetFileSizeEx(FileHandle, &FileSize))
			{
				uint32 FileSize32 = SafeTruncateUint64(FileSize.QuadPart);
				result.data = GameMemory::PushTransientCount<char>(FileSize32);
				if (result.data)
				{
					if (!metaDataOnly)
					{
						DWORD BytesRead;

						if (ReadFile(FileHandle, result.data, FileSize32, &BytesRead, 0) &&
							(FileSize32 == BytesRead))
						{
							result.sizeBytes = FileSize32;
						}
						else
						{
							// TODO: Logging
						}
					}

					FILETIME creationTime;
					FILETIME lastAcessTime;
					FILETIME lastWriteTime;
					if (GetFileTime(FileHandle, &creationTime, &lastAcessTime, &lastWriteTime))
					{
						ULARGE_INTEGER large = {};

						large.LowPart = creationTime.dwLowDateTime;
						large.HighPart = creationTime.dwHighDateTime;
						result.creationTime = large.QuadPart;

						large.LowPart = lastAcessTime.dwLowDateTime;
						large.HighPart = lastAcessTime.dwHighDateTime;
						result.lastAcessTime = large.QuadPart;

						large.LowPart = lastWriteTime.dwLowDateTime;
						large.HighPart = lastWriteTime.dwHighDateTime;
						result.lastWriteTime = large.QuadPart;
					}
					else
					{
						SOLWARN(String("Could not get meta data for file: ").Add(path).GetCStr());
					}
				}
				else
				{
					SOLERROR(String("Could not get allocate for file: ").Add(path).GetCStr());
				}
			}
			else
			{
				SOLERROR(String("Could not get size for file: ").Add(path).GetCStr());
			}

			CloseHandle(FileHandle);
		}
		else
		{
			SOLERROR(String("Could not open file: ").Add(path).GetCStr());
		}

		return result;
	}

	void InitializeClock()
	{
		LARGE_INTEGER frequency = {};
		QueryPerformanceFrequency(&frequency);
		winState.clockFrequency = 1.0 / (real64)frequency.QuadPart;
		QueryPerformanceCounter(&winState.startTime);
	}

	void Platform::Quit()
	{
		winState.running = false;
	}

	uint32 Platform::GetWindowWidth()
	{
		return winState.width;
	}

	uint32 Platform::GetWindowHeight()
	{
		return winState.height;
	}

	void Platform::ConsoleWrite(const char* message, uint8 colour)
	{
		HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
		// @NOTE: FATAL, ERROR, WARN, INFO, DEBUG, TRACE
		static uint8 levels[6] = { 64, 4, 6, 2, 1, 8 };
		SetConsoleTextAttribute(console_handle, levels[colour]);
		OutputDebugStringA(message);
		uint64 length = strlen(message);
		LPDWORD number_written = 0;
		WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), message, (DWORD)length, number_written, 0);
	}

	real64 Platform::GetAbsoluteTime()
	{
		if (!winState.clockFrequency) {
			InitializeClock();
		}

		LARGE_INTEGER nowTime = {};
		QueryPerformanceCounter(&nowTime);

		return (real64)nowTime.QuadPart * winState.clockFrequency;
	}

	static void ProcessMouseInput()
	{
		Input* input = Input::Get();

		POINT mousep = {};
		GetCursorPos(&mousep);
		ScreenToClient((HWND)winState.window, &mousep);
		real32 mx = (real32)mousep.x;
		real32 my = (real32)mousep.y;

		mx = Clamp<real32>(mx, 0.0f, (real32)winState.width);
		my = Clamp<real32>(my, 0.0f, (real32)winState.height);

		input->mousePositionPixelCoords.x = mx;
		input->mousePositionPixelCoords.y = my;

		if (input->mouse_locked && winState.active)
		{
			SetCursor(FALSE);

			input->oldInput->mousePositionPixelCoords = Vec2f((real32)(winState.width / 2),
				(real32)(winState.height / 2));

			POINT p = {};
			p.x = winState.width / 2;
			p.y = winState.height / 2;

			ClientToScreen((HWND)winState.window, &p);

			SetCursorPos(p.x, p.y);
		}

		input->mouseNorm.x = mx / (real32)winState.width;
		input->mouseNorm.y = my / (real32)winState.height;

		input->shift = (GetKeyState(VK_SHIFT) & (1 << 15));
		input->alt = (GetKeyState(VK_MENU) & (1 << 15));
		input->ctrl = (GetKeyState(VK_CONTROL) & (1 << 15));

		input->mb1 = GetKeyState(VK_LBUTTON) & (1 << 15);
		input->mb2 = GetKeyState(VK_RBUTTON) & (1 << 15);
		input->mb3 = GetKeyState(VK_MBUTTON) & (1 << 15);

		input->mouseDelta = input->mousePositionPixelCoords - input->oldInput->mousePositionPixelCoords;
	}

	bool8 Platform::PumpMessages()
	{
		if (winState.running)
		{
			Input::Flip();

			winState.active = (bool8)GetFocus();
			MSG message = {};
			while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&message);
				DispatchMessageA(&message);
			}

			ProcessMouseInput();
		}

		return winState.running;
	}

	void* Platform::GetNativeState()
	{
		return winState.window;
	}

	LRESULT CALLBACK WindProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	bool8 Platform::Intialize(int32 x, int32 y, int32 width, int32 height)
	{
		winState.hinstance = GetModuleHandleA(0);

		WNDCLASSEXA windowClass = {};
		windowClass.cbSize = sizeof(windowClass);
		windowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
		windowClass.lpfnWndProc = &WindProc;
		windowClass.cbClsExtra = 0;
		windowClass.cbWndExtra = 0;
		windowClass.hInstance = 0;
		windowClass.hIcon = LoadIcon(winState.hinstance, IDI_APPLICATION);
		windowClass.hIconSm = nullptr;
		windowClass.hCursor = nullptr;
		windowClass.hbrBackground = nullptr;
		windowClass.lpszClassName = "GaMe eNgInE";
		if (RegisterClassExA(&windowClass))
		{
			// @NOTE: Create window
			uint32 clientX = x;
			uint32 clientY = y;
			uint32 clientWidth = width;
			uint32 clientHeight = height;

			uint32 windowX = clientX;
			uint32 windowY = clientY;
			uint32 windowWidth = clientWidth;
			uint32 windowHeight = clientHeight;

			uint32 windowStyle = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
			uint32 windowExStyle = WS_EX_APPWINDOW;

			windowStyle |= WS_MAXIMIZEBOX;
			windowStyle |= WS_MINIMIZEBOX;
			windowStyle |= WS_THICKFRAME;

			// @NOTE: Obtain the size of the border.
			RECT border_rect = { 0, 0, 0, 0 };
			AdjustWindowRectEx(&border_rect, windowStyle, 0, windowExStyle);

			// @NOTE: In this case, the border rectangle is negative.
			windowX += border_rect.left;
			windowY += border_rect.top;

			// @NOTE: Grow by the size of the OS border.
			windowWidth += border_rect.right - border_rect.left;
			windowHeight += border_rect.bottom - border_rect.top;

			winState.window = CreateWindowExA(
				NULL, windowClass.lpszClassName, "GaMe eNgInE", windowStyle,
				windowX, windowY, windowWidth, windowHeight,
				NULL, NULL, winState.hinstance, NULL);

			if (winState.window)
			{
				SOLINFO("Win32 Window created and running");
				ShowWindow(winState.window, SW_SHOW);
				winState.running = true;
				winState.width = windowWidth;
				winState.height = windowHeight;

				//InitializeRawInput(ps);
				InitializeClock();
			}
			else
			{
				MessageBoxA(NULL, "Window creation failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
				SOLFATAL("Window creation failed!");
				return false;
			}
		}
		else
		{
			MessageBoxA(0, "Window registration failed", "Error", MB_ICONEXCLAMATION | MB_OK);
			SOLFATAL("Window registration failed");
			return false;
		}

		return true;
	}

	void Platform::Shutdown()
	{

	}

	void ProcessKeyboardInput(uint16 vkCode, bool32 isDown);
	LRESULT CALLBACK WindProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		LRESULT result = {};

		Win32EventPumpMessageContext eventContext = {};
		eventContext.hwnd = hwnd;
		eventContext.msg = msg;
		eventContext.wparam = wparam;
		eventContext.lparam = lparam;
		if (EventSystem::Fire((uint16)EngineEvent::Value::WINDOW_PUMP_MESSAGES, &result, eventContext))
		{
			if (result)
			{
				return result;
			}
		}

		switch (msg)
		{
		case WM_DESTROY:
		{
			winState.running = false;
			PostQuitMessage(0);
		} break;
		case WM_CLOSE:
		{
			winState.running = false;
			PostQuitMessage(0);
		} break;
		case WM_SIZE:
		{
			RECT r = {};
			GetClientRect(hwnd, &r);
			EventWindowResize eventResize = {};
			eventResize.width = r.right - r.left;
			eventResize.height = r.bottom - r.top;
			winState.width = eventResize.width;
			winState.height = eventResize.height;

			EventSystem::Fire<EventWindowResize>((uint16)EngineEvent::Value::WINDOW_RESIZED, 0, eventResize);

		} break;
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYDOWN:
		case WM_KEYUP:
		{
			uint32 vkCode = (uint32)wparam;
			bool32 isDown = ((lparam & ((int64)1 << (int64)31)) == 0);
			ProcessKeyboardInput(vkCode, isDown);
		}break;
		default: { result = DefWindowProcA(hwnd, msg, wparam, lparam);	}
		}

		return result;
	}

	static void ProcessKeyboardInput(uint16 vkCode, bool32 isDown)
	{
		Input* input = Input::Get();

		if (vkCode == 'W')
		{
			input->w = isDown;
		}
		else if (vkCode == 'A')
		{
			input->a = isDown;
		}
		else if (vkCode == 'S')
		{
			input->s = isDown;
		}
		else if (vkCode == 'D')
		{
			input->d = isDown;
		}
		else if (vkCode == 'Q')
		{
			input->q = isDown;
		}
		else if (vkCode == 'E')
		{
			input->e = isDown;
		}
		else if (vkCode == 'R')
		{
			input->r = isDown;
		}
		else if (vkCode == 'T')
		{
			input->t = isDown;
		}
		else if (vkCode == 'Z')
		{
			input->z = isDown;
		}
		else if (vkCode == 'X')
		{
			input->x = isDown;
		}
		else if (vkCode == 'C')
		{
			input->c = isDown;
		}
		else if (vkCode == 'V')
		{
			input->v = isDown;
		}
		else if (vkCode == 'B')
		{
			input->b = isDown;
		}
		else if (vkCode == '0')
		{
			input->K0 = isDown;
		}
		else if (vkCode == '1')
		{
			input->K1 = isDown;
		}
		else if (vkCode == '2')
		{
			input->K2 = isDown;
		}
		else if (vkCode == '3')
		{
			input->K3 = isDown;
		}
		else if (vkCode == '4')
		{
			input->K4 = isDown;
		}
		else if (vkCode == '5')
		{
			input->K5 = isDown;
		}
		else if (vkCode == '6')
		{
			input->K6 = isDown;
		}
		else if (vkCode == '7')
		{
			input->K7 = isDown;
		}
		else if (vkCode == '8')
		{
			input->K8 = isDown;
		}
		else if (vkCode == '9')
		{
			input->K9 = isDown;
		}
		else if (vkCode == VK_F1)
		{
			input->f1 = isDown;
		}
		else if (vkCode == VK_F2)
		{
			input->f2 = isDown;
		}
		else if (vkCode == VK_F3)
		{
			input->f3 = isDown;
		}
		else if (vkCode == VK_F4)
		{
			input->f4 = isDown;
		}
		else if (vkCode == VK_F5)
		{
			input->f5 = isDown;
		}
		else if (vkCode == VK_F6)
		{
			input->f6 = isDown;
		}
		else if (vkCode == VK_F6)
		{
			input->f7 = isDown;
		}
		else if (vkCode == VK_F7)
		{
			input->f7 = isDown;
		}
		else if (vkCode == VK_F8)
		{
			input->f8 = isDown;
		}
		else if (vkCode == VK_F9)
		{
			input->f9 = isDown;
		}
		else if (vkCode == VK_F10)
		{
			input->f10 = isDown;
		}
		else if (vkCode == VK_F11)
		{
			input->f11 = isDown;
		}
		else if (vkCode == VK_F12)
		{
			input->f12 = isDown;
		}
		else if (vkCode == VK_ESCAPE)
		{
			input->escape = isDown;
		}
		else if (vkCode == VK_OEM_3)
		{
			input->tlda = isDown;
		}
		else if (vkCode == VK_DELETE)
		{
			input->del = isDown;
		}
		else if (vkCode == VK_SHIFT)
		{
			input->shift = isDown;
		}
		else if (vkCode == VK_MENU)
		{
			input->alt = isDown;
		}
		else if (vkCode == VK_CONTROL)
		{
			input->ctrl = isDown;
		}
		else if (vkCode == VK_SPACE)
		{
			input->space = isDown;
		}
	}
}
#endif