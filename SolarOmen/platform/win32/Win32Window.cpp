#include "Win32Window.h"
#include "core/SolarPlatform.h"

#include <shobjidl.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "../../vendor/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace cm
{
	static bool platformRunning = true;

	bool32 Platform::WriteFile(const CString& path, void* data, uint32 sizeBytes)
	{
		bool32 result = false;

		HANDLE FileHandle = CreateFileA(path.GetCStr(), GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
		if (FileHandle != INVALID_HANDLE_VALUE)
		{
			DWORD BytesWritten;

			if (::WriteFile(FileHandle, data, SafeTruncateUint64(sizeBytes), &BytesWritten, 0))
			{

				result = (BytesWritten == sizeBytes);
			}
			else
			{
				// TODO(casey): Logging
			}

			CloseHandle(FileHandle);
		}
		else
		{
			// TODO(casey): Logging
		}

		return result;
	}

	// @TODO: Remove std::vector!!
	ManagedArray<CString> Platform::LoadEntireFolder(const CString& path, const CString& fileTypes)
	{
		WIN32_FIND_DATAA fdFile = {};
		HANDLE file = {};

		std::vector<CString> result;
		CString searchString = {};
		searchString.Add(path).Add('*');

		if ((file = FindFirstFileA(searchString.GetCStr(), &fdFile)) != INVALID_HANDLE_VALUE)
		{
			do
			{
				if (strcmp(fdFile.cFileName, ".") != 0
					&& strcmp(fdFile.cFileName, "..") != 0)
				{
					CString filePath = CString(path).Add(CString(fdFile.cFileName));
					// @NOTE: Is the entity a File or Folder? 
					if (fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					{
						ManagedArray<CString> subFolder = Platform::LoadEntireFolder(filePath.Add('/'), fileTypes);

						for (uint32 i = 0; i < subFolder.GetCount(); i++)
						{
							result.push_back(subFolder[i]);
						}
					}
					else
					{
						CString ext = Util::GetFileExtension(filePath);
						if (ext == fileTypes)
						{
							result.push_back(filePath);
						}
					}
				}
			} while (FindNextFileA(file, &fdFile) != 0);
		}
		else
		{
			LOG("No files found");
		}

		if (file)
		{
			FindClose(file);
		}

		ManagedArray<CString> paths = {};
		paths.data = GameMemory::PushTransientCount<CString>((uint32)result.size());
		paths.capcity = (uint32)result.size();

		for (const CString& path : result)
		{
			paths.Add(path);
		}

		return paths;
	}

	PlatformFile Platform::LoadEntireFile(const CString& path, bool32 metaDataOnly)
	{
		PlatformFile result = {};
		result.path = path;

		HANDLE FileHandle = CreateFileA(path.GetCStr(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
		if (FileHandle != INVALID_HANDLE_VALUE)
		{
			LARGE_INTEGER FileSize;
			if (GetFileSizeEx(FileHandle, &FileSize))
			{
				uint32 FileSize32 = SafeTruncateUInt64(FileSize.QuadPart);
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
						// TODO: Logging
					}
				}
				else
				{
					// TODO: Logging
				}
			}
			else
			{
				// TODO: Logging
			}

			CloseHandle(FileHandle);
		}
		else
		{
			// TODO: Logging
		}

		return result;
	}

	LRESULT CALLBACK WindProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		LRESULT result = {};
#if EDITOR
		result = ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam);
		if (result)
			return result;
#endif

		switch (msg)
		{
		case WM_DESTROY:
		{
			platformRunning = false;
			PostQuitMessage(0);
		}
		break;

		case WM_CLOSE:
		{
			platformRunning = false;
			PostQuitMessage(0);
		}
		break;

		case WM_MOUSEMOVE:
		{
		}
		break;

		default:
		{
			result = DefWindowProcA(hwnd, msg, wparam, lparam);
		}
		}

		return result;
	}

	void InitializeRawInput(PlatformState* win)
	{
		// @NOTE: Mouse
		{
			RAWINPUTDEVICE rid = {};
			rid.usUsagePage = 0x1;
			rid.usUsage = 0x02;
			rid.dwFlags = 0;
			rid.hwndTarget = NULL; // @NOTE: I think this means listen to events outside of window

			if (RegisterRawInputDevices(&rid, 1, sizeof(rid)))
			{
				win->rawInput = true;
			}
			else
			{
				win->rawInput = false;
				LOG("Could not init raw input");
			}
		}

		// @NOTE: Keyboard
		{
			RAWINPUTDEVICE rid = {};
			rid.usUsagePage = 0x1;
			rid.usUsage = 0x06;
			rid.dwFlags = 0;
			rid.hwndTarget = NULL; // @NOTE: I think this means listen to events outside of window

			if (RegisterRawInputDevices(&rid, 1, sizeof(rid)))
			{
				win->rawInput = true;
			}
			else
			{
				win->rawInput = false;
				LOG("Could not init raw input");
			}
		}
	}

	static void ProcessKeyboardInput(uint16 vkCode, bool32 isDown, Input* input)
	{
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

	static void ProcessRawInput(LPARAM lparam, Input* input)
	{
		uint32 size;
		GetRawInputData(reinterpret_cast<HRAWINPUT>(lparam), RID_INPUT, NULL, &size, sizeof(RAWINPUTHEADER));

		if (size > 0)
		{
			std::vector<uint8> data(size);
			uint32 read = GetRawInputData(reinterpret_cast<HRAWINPUT>(lparam), RID_INPUT, data.data(), &size, sizeof(RAWINPUTHEADER));

			if (read == size)
			{
				RAWINPUT* rawInput = reinterpret_cast<RAWINPUT*>(data.data());
				if (rawInput->header.dwType == RIM_TYPEMOUSE)
				{
					real32 x = static_cast<real32>(rawInput->data.mouse.lLastX);
					real32 y = static_cast<real32>(rawInput->data.mouse.lLastY);

					input->mouseDelta += Vec2f(x, y);
				}
				else if (rawInput->header.dwType == RIM_TYPEKEYBOARD)
				{
					uint16 vkCode = rawInput->data.keyboard.VKey;
					bool32 isDown = false;
					if (rawInput->data.keyboard.Flags == RI_KEY_MAKE)
					{
						isDown = true;
					}
					else //else if (rawInput->data.keyboard.Flags == RI_KEY_BREAK)
					{
						isDown = false;
					}

					ProcessKeyboardInput(vkCode, isDown, input);
				}
			}
		}
	}

	void cm::Platform::PostQuitMessage()
	{
		platformRunning = false;
	}

	static void Win32ProcessPendingMessages(PlatformState* win_state, Input* input)
	{
		win_state->isFocused = (bool)GetFocus();
		if (win_state->rawInput)
		{
			input->mouseDelta = Vec2f(0);

			MSG msg = {};
			while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE))
			{
				switch (msg.message)
				{
				case WM_INPUT:
				{
					ProcessRawInput(msg.lParam, input);
				}break;

				default:
				{
					TranslateMessage(&msg);
					DispatchMessageA(&msg);
				} break;
				}
			}

			POINT mousep = {};
			GetCursorPos(&mousep);
			ScreenToClient((HWND)win_state->window, &mousep);
			real32 mx = (real32)mousep.x;
			real32 my = (real32)mousep.y;

			mx = Clamp<real32>(mx, 0.0f, (real32)win_state->clientWidth);
			my = Clamp<real32>(my, 0.0f, (real32)win_state->clientHeight);

			input->mousePositionPixelCoords.x = mx;
			input->mousePositionPixelCoords.y = my;

			if (input->mouse_locked && win_state->isFocused)
			{
				SetCursor(FALSE);

				input->old_input->mousePositionPixelCoords = Vec2f((real32)(win_state->clientWidth / 2),
					(real32)(win_state->clientHeight / 2));

				POINT p = {};
				p.x = win_state->clientWidth / 2;
				p.y = win_state->clientHeight / 2;

				ClientToScreen((HWND)win_state->window, &p);

				SetCursorPos(p.x, p.y);
			}

			input->del = (GetKeyState(VK_DELETE) & (1 << 15));

			input->mouse_norm.x = mx / (real32)win_state->clientWidth;
			input->mouse_norm.y = my / (real32)win_state->clientHeight;

			input->mb1 = GetKeyState(VK_LBUTTON) & (1 << 15);
			input->mb2 = GetKeyState(VK_RBUTTON) & (1 << 15);
			input->mb3 = GetKeyState(VK_MBUTTON) & (1 << 15);
		}
		else
		{
			MSG msg = {};
			while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE))
			{
				switch (msg.message)
				{
				case WM_SYSKEYDOWN:
				case WM_SYSKEYUP:
				case WM_KEYDOWN:
				case WM_KEYUP:
				{
					uint32 vkCode = (uint32)msg.wParam;

					bool32 alt_key_was_down = (msg.lParam & (1 << 29));
					bool32 shift_key_was_down = (GetKeyState(VK_SHIFT) & (1 << 15));

					bool32 was_down = ((msg.lParam & (1 << 30)) != 0);
					bool32 isDown = ((msg.lParam & (1 << 31)) == 0);

					ProcessKeyboardInput(vkCode, isDown, input);
				}break;

				default:
				{
					TranslateMessage(&msg);
					DispatchMessageA(&msg);
				} break;
				}
			}

			POINT mousep = {};
			GetCursorPos(&mousep);
			ScreenToClient((HWND)win_state->window, &mousep);
			real32 mx = (real32)mousep.x;
			real32 my = (real32)mousep.y;

			mx = Clamp<real32>(mx, 0.0f, (real32)win_state->clientWidth);
			my = Clamp<real32>(my, 0.0f, (real32)win_state->clientHeight);

			input->mousePositionPixelCoords.x = mx;
			input->mousePositionPixelCoords.y = my;

			if (input->mouse_locked && win_state->isFocused)
			{
				SetCursor(FALSE);

				input->old_input->mousePositionPixelCoords = Vec2f((real32)(win_state->clientWidth / 2),
					(real32)(win_state->clientHeight / 2));

				POINT p = {};
				p.x = win_state->clientWidth / 2;
				p.y = win_state->clientHeight / 2;

				ClientToScreen((HWND)win_state->window, &p);

				SetCursorPos(p.x, p.y);
			}

			input->mouse_norm.x = mx / (real32)win_state->clientWidth;
			input->mouse_norm.y = my / (real32)win_state->clientHeight;

			input->shift = (GetKeyState(VK_SHIFT) & (1 << 15));
			input->alt = (GetKeyState(VK_MENU) & (1 << 15));
			input->ctrl = (GetKeyState(VK_CONTROL) & (1 << 15));

			input->mb1 = GetKeyState(VK_LBUTTON) & (1 << 15);
			input->mb2 = GetKeyState(VK_RBUTTON) & (1 << 15);
			input->mb3 = GetKeyState(VK_MBUTTON) & (1 << 15);

			input->mouseDelta = input->mousePositionPixelCoords - input->old_input->mousePositionPixelCoords;
		}

		if (!win_state->isFocused)
		{
			Input* oldInput = input->old_input;
			ZeroStruct(input);
			input->old_input = oldInput;
		}
	}

	bool32 cm::Platform::ProcessInput(PlatformState* ps, Input* input)
	{
		Win32ProcessPendingMessages(ps, input);

		return platformRunning;
	}

	bool32 cm::Platform::Initialize(PlatformState* ps, const char* title, int32 width, int32 height, bool32 createConsole)
	{
		if (createConsole)
		{
			AllocConsole();
			AttachConsole(GetCurrentProcessId());
			FILE* stream;
			freopen_s(&stream, "CONOUT$", "w+", stdout);
			freopen_s(&stream, "CONOUT$", "w+", stderr);

			HWND console = GetConsoleWindow();
			SetWindowPos(console, 0, 1920 / 2 + 1000, 1080 / 2 - 250, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

			ps->console = (int64)console;
		}

		RECT client_rect;
		client_rect.left = 0;
		client_rect.right = (LONG)width;
		client_rect.top = 0;
		client_rect.bottom = (LONG)height;

		DWORD window_style = WS_VISIBLE | (WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME);
		AdjustWindowRect(&client_rect, window_style, FALSE);

		WNDCLASSEXA window_class = {};
		window_class.cbSize = sizeof(window_class);
		window_class.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
		window_class.lpfnWndProc = &WindProc;
		window_class.cbClsExtra = 0;
		window_class.cbWndExtra = 0;
		window_class.hInstance = 0;
		window_class.hIcon = nullptr;
		window_class.hIconSm = nullptr;
		window_class.hCursor = nullptr;
		window_class.hbrBackground = nullptr;
		window_class.lpszClassName = title;

		RegisterClassExA(&window_class);

		HWND window = CreateWindowExA(
			NULL, window_class.lpszClassName, title, window_style,
			CW_USEDEFAULT, CW_USEDEFAULT, client_rect.right - client_rect.left, client_rect.bottom - client_rect.top,
			NULL, NULL, NULL, NULL);

		GetClientRect(window, &client_rect);

		ps->window = (int64)window;
		ps->clientWidth = client_rect.right - client_rect.left;
		ps->clientHeight = client_rect.bottom - client_rect.top;
		ps->aspect = (real32)ps->clientWidth / (real32)ps->clientHeight;

		if (window)
		{
			// NOTE: Adjust window pos ect
			int32 window_pos_x = GetSystemMetrics(SM_CXSCREEN) / 2 - ps->clientWidth / 2;
			int32 window_pos_y = GetSystemMetrics(SM_CYSCREEN) / 2 - ps->clientHeight / 2;
			SetWindowPos(window, NULL, window_pos_x, window_pos_y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
			//================================================================//

			// NOTE: Initialize raw input --- duh
			InitializeRawInput(ps);
			//================================================================//

			platformRunning = true;

			PlatformState::Initialize(ps);

			return true;
		}

		return false;
	}

	void cm::Platform::Shutdown()
	{
		// TODO:
	}
}