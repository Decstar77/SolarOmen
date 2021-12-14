#include "Win32Window.h"
#include "core/SolarPlatform.h"

#include <shobjidl.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Xinput.h>

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
			RAWINPUTDEVICE mouseRID = {};
			mouseRID.usUsagePage = 0x1;
			mouseRID.usUsage = 0x02;
			mouseRID.dwFlags = 0;
			mouseRID.hwndTarget = (HWND)win->window;

			RAWINPUTDEVICE ps4ControllerRID;
			ps4ControllerRID.usUsagePage = 0x01;
			ps4ControllerRID.usUsage = 0x05;
			ps4ControllerRID.dwFlags = RIDEV_INPUTSINK;
			ps4ControllerRID.hwndTarget = (HWND)win->window;;

			if (RegisterRawInputDevices(&mouseRID, 1, sizeof(mouseRID)) &&
				RegisterRawInputDevices(&ps4ControllerRID, 1, sizeof(ps4ControllerRID)))
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

	struct PS4OutputData
	{
		BYTE buffer[96];
		HANDLE file;
		OVERLAPPED overlapped;
	};

	uint32_t MakeReflectedCRC32(BYTE* data, uint32_t byteCount)
	{
		static const uint32_t crcTable[] = {
			0x00000000,0x77073096,0xEE0E612C,0x990951BA,0x076DC419,0x706AF48F,0xE963A535,0x9E6495A3,0x0EDB8832,0x79DCB8A4,0xE0D5E91E,0x97D2D988,0x09B64C2B,
			0x7EB17CBD,0xE7B82D07,0x90BF1D91,0x1DB71064,0x6AB020F2,0xF3B97148,0x84BE41DE,0x1ADAD47D,0x6DDDE4EB,0xF4D4B551,0x83D385C7,0x136C9856,0x646BA8C0,
			0xFD62F97A,0x8A65C9EC,0x14015C4F,0x63066CD9,0xFA0F3D63,0x8D080DF5,0x3B6E20C8,0x4C69105E,0xD56041E4,0xA2677172,0x3C03E4D1,0x4B04D447,0xD20D85FD,
			0xA50AB56B,0x35B5A8FA,0x42B2986C,0xDBBBC9D6,0xACBCF940,0x32D86CE3,0x45DF5C75,0xDCD60DCF,0xABD13D59,0x26D930AC,0x51DE003A,0xC8D75180,0xBFD06116,
			0x21B4F4B5,0x56B3C423,0xCFBA9599,0xB8BDA50F,0x2802B89E,0x5F058808,0xC60CD9B2,0xB10BE924,0x2F6F7C87,0x58684C11,0xC1611DAB,0xB6662D3D,0x76DC4190,
			0x01DB7106,0x98D220BC,0xEFD5102A,0x71B18589,0x06B6B51F,0x9FBFE4A5,0xE8B8D433,0x7807C9A2,0x0F00F934,0x9609A88E,0xE10E9818,0x7F6A0DBB,0x086D3D2D,
			0x91646C97,0xE6635C01,0x6B6B51F4,0x1C6C6162,0x856530D8,0xF262004E,0x6C0695ED,0x1B01A57B,0x8208F4C1,0xF50FC457,0x65B0D9C6,0x12B7E950,0x8BBEB8EA,
			0xFCB9887C,0x62DD1DDF,0x15DA2D49,0x8CD37CF3,0xFBD44C65,0x4DB26158,0x3AB551CE,0xA3BC0074,0xD4BB30E2,0x4ADFA541,0x3DD895D7,0xA4D1C46D,0xD3D6F4FB,
			0x4369E96A,0x346ED9FC,0xAD678846,0xDA60B8D0,0x44042D73,0x33031DE5,0xAA0A4C5F,0xDD0D7CC9,0x5005713C,0x270241AA,0xBE0B1010,0xC90C2086,0x5768B525,
			0x206F85B3,0xB966D409,0xCE61E49F,0x5EDEF90E,0x29D9C998,0xB0D09822,0xC7D7A8B4,0x59B33D17,0x2EB40D81,0xB7BD5C3B,0xC0BA6CAD,0xEDB88320,0x9ABFB3B6,
			0x03B6E20C,0x74B1D29A,0xEAD54739,0x9DD277AF,0x04DB2615,0x73DC1683,0xE3630B12,0x94643B84,0x0D6D6A3E,0x7A6A5AA8,0xE40ECF0B,0x9309FF9D,0x0A00AE27,
			0x7D079EB1,0xF00F9344,0x8708A3D2,0x1E01F268,0x6906C2FE,0xF762575D,0x806567CB,0x196C3671,0x6E6B06E7,0xFED41B76,0x89D32BE0,0x10DA7A5A,0x67DD4ACC,
			0xF9B9DF6F,0x8EBEEFF9,0x17B7BE43,0x60B08ED5,0xD6D6A3E8,0xA1D1937E,0x38D8C2C4,0x4FDFF252,0xD1BB67F1,0xA6BC5767,0x3FB506DD,0x48B2364B,0xD80D2BDA,
			0xAF0A1B4C,0x36034AF6,0x41047A60,0xDF60EFC3,0xA867DF55,0x316E8EEF,0x4669BE79,0xCB61B38C,0xBC66831A,0x256FD2A0,0x5268E236,0xCC0C7795,0xBB0B4703,
			0x220216B9,0x5505262F,0xC5BA3BBE,0xB2BD0B28,0x2BB45A92,0x5CB36A04,0xC2D7FFA7,0xB5D0CF31,0x2CD99E8B,0x5BDEAE1D,0x9B64C2B0,0xEC63F226,0x756AA39C,
			0x026D930A,0x9C0906A9,0xEB0E363F,0x72076785,0x05005713,0x95BF4A82,0xE2B87A14,0x7BB12BAE,0x0CB61B38,0x92D28E9B,0xE5D5BE0D,0x7CDCEFB7,0x0BDBDF21,
			0x86D3D2D4,0xF1D4E242,0x68DDB3F8,0x1FDA836E,0x81BE16CD,0xF6B9265B,0x6FB077E1,0x18B74777,0x88085AE6,0xFF0F6A70,0x66063BCA,0x11010B5C,0x8F659EFF,
			0xF862AE69,0x616BFFD3,0x166CCF45,0xA00AE278,0xD70DD2EE,0x4E048354,0x3903B3C2,0xA7672661,0xD06016F7,0x4969474D,0x3E6E77DB,0xAED16A4A,0xD9D65ADC,
			0x40DF0B66,0x37D83BF0,0xA9BCAE53,0xDEBB9EC5,0x47B2CF7F,0x30B5FFE9,0xBDBDF21C,0xCABAC28A,0x53B39330,0x24B4A3A6,0xBAD03605,0xCDD70693,0x54DE5729,
			0x23D967BF,0xB3667A2E,0xC4614AB8,0x5D681B02,0x2A6F2B94,0xB40BBE37,0xC30C8EA1,0x5A05DF1B,0x2D02EF8D
		};

		uint32_t crc = (uint32_t)~0;
		while (byteCount--)
		{
			crc = (crc >> 8) ^ crcTable[(uint8_t)crc ^ *data];
			++data;
		}
		return ~crc;
	}

	static void UpdateDualshock4(BYTE rawData[], DWORD byteCount, WCHAR* deviceName, PS4OutputData* outputData)
	{
		const DWORD usbInputByteCount = 64;
		const DWORD bluetoothInputByteCount = 547;

		int offset = 0;
		if (byteCount == bluetoothInputByteCount) offset = 2;

		BYTE leftStickX = rawData[offset + 1];
		BYTE leftStickY = rawData[offset + 2];
		BYTE rightStickX = rawData[offset + 3];
		BYTE rightStickY = rawData[offset + 4];
		BYTE leftTrigger = rawData[offset + 8];
		BYTE rightTrigger = rawData[offset + 9];
		BYTE dpad = 0b1111 & rawData[offset + 5];
		printf("DS4 - LX:%3d LY:%3d RX:%3d RY:%3d LT:%3d RT:%3d Dpad:%1d ", leftStickX, leftStickY, rightStickX, rightStickY, leftTrigger, rightTrigger, dpad);

		BYTE battery = rawData[offset + 12];
		int16_t touch1X = ((rawData[offset + 37] & 0x0F) << 8) | rawData[offset + 36];
		int16_t touch1Y = ((rawData[offset + 37]) >> 4) | (rawData[offset + 38] << 4);
		int16_t touch2X = ((rawData[offset + 41] & 0x0F) << 8) | rawData[offset + 40];
		int16_t touch2Y = ((rawData[offset + 41]) >> 4) | (rawData[offset + 42] << 4);
		printf("Battery:%3d Touch1X:%4d Touch1Y:%4d Touch2X:%4d Touch2Y:%4d ", battery, touch1X, touch1Y, touch2X, touch2Y);

		printf("Buttons: ");
		if (1 & (rawData[offset + 5] >> 4)) printf("Square ");
		if (1 & (rawData[offset + 5] >> 5)) printf("X ");
		if (1 & (rawData[offset + 5] >> 6)) printf("O ");
		if (1 & (rawData[offset + 5] >> 7)) printf("Triangle ");
		if (1 & (rawData[offset + 6] >> 0)) printf("L1 ");
		if (1 & (rawData[offset + 6] >> 1)) printf("R1 ");
		if (1 & (rawData[offset + 6] >> 2)) printf("L2 ");
		if (1 & (rawData[offset + 6] >> 3)) printf("R2 ");
		if (1 & (rawData[offset + 6] >> 4)) printf("Share ");
		if (1 & (rawData[offset + 6] >> 5)) printf("Options ");
		if (1 & (rawData[offset + 6] >> 6)) printf("L3 ");
		if (1 & (rawData[offset + 6] >> 7)) printf("R3 ");
		if (1 & (rawData[offset + 7] >> 0)) printf("PS ");
		if (1 & (rawData[offset + 7] >> 1)) printf("TouchPad ");
		printf("\n");

		// Ouput force-feedback and LED color
		int headerSize = 0;
		int outputByteCount = 0;
		if (byteCount == usbInputByteCount)
		{
			outputByteCount = 32;
			outputData->buffer[0] = 0x05;
			outputData->buffer[1] = 0xFF;
		}
		if (byteCount == bluetoothInputByteCount)
		{
			outputByteCount = 78;
			outputData->buffer[0] = 0xA2; // Header - Bluetooth HID report type: data/output
			outputData->buffer[1] = 0x11;
			outputData->buffer[2] = 0XC0;
			outputData->buffer[4] = 0x07;
			headerSize = 1;
		}

		BYTE lightRumble = rightTrigger;
		BYTE heavyRumble = leftTrigger;
		BYTE ledRed = (BYTE)(touch1X * 255 / 2000);
		BYTE ledGreen = (BYTE)(touch1Y * 255 / 1000);
		BYTE ledBlue = (BYTE)(touch2Y * 255 / 1000);

		outputData->buffer[4 + offset + headerSize] = lightRumble;
		outputData->buffer[5 + offset + headerSize] = heavyRumble;
		outputData->buffer[6 + offset + headerSize] = ledRed;
		outputData->buffer[7 + offset + headerSize] = ledGreen;
		outputData->buffer[8 + offset + headerSize] = ledBlue;

		if (byteCount == bluetoothInputByteCount)
		{
			uint32_t crc = MakeReflectedCRC32(outputData->buffer, 75);
			memcpy(outputData->buffer + 75, &crc, sizeof(crc));
		}

		DWORD bytesTransferred;
		bool finishedLastOutput = GetOverlappedResult(outputData->file, &outputData->overlapped, &bytesTransferred, false);
		if (finishedLastOutput)
		{
			if (outputData->file) CloseHandle(outputData->file);
			outputData->file = CreateFileW(deviceName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
			if (outputData->file != INVALID_HANDLE_VALUE)
			{
				WriteFile(outputData->file, (void*)(outputData->buffer + headerSize), outputByteCount, 0, &outputData->overlapped);
			}
		}
	}

	static bool IsDualshock4(RID_DEVICE_INFO_HID info)
	{
		const DWORD sonyVendorID = 0x054C;
		const DWORD ds4Gen1ProductID = 0x05C4;
		const DWORD ds4Gen2ProductID = 0x09CC;

		return info.dwVendorId == sonyVendorID && (info.dwProductId == ds4Gen1ProductID || info.dwProductId == ds4Gen2ProductID);
	}

	static void ProcessRawInput(LPARAM lparam, Input* input)
	{
		uint32 size = 0;
		GetRawInputData(reinterpret_cast<HRAWINPUT>(lparam), RID_INPUT, NULL, &size, sizeof(RAWINPUTHEADER));

		if (size > 0)
		{
			uint8* data = GameMemory::PushTransientCount<uint8>(size);
			uint32 read = GetRawInputData(reinterpret_cast<HRAWINPUT>(lparam), RID_INPUT, data, &size, sizeof(RAWINPUTHEADER));

			if (read == size)
			{
				RAWINPUT* rawInput = reinterpret_cast<RAWINPUT*>(data);
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
				else if (rawInput->header.dwType == RIM_TYPEHID)
				{
					RID_DEVICE_INFO deviceInfo;
					UINT deviceInfoSize = sizeof(deviceInfo);
					bool gotInfo = GetRawInputDeviceInfo(rawInput->header.hDevice, RIDI_DEVICEINFO, &deviceInfo, &deviceInfoSize) > 0;

					WCHAR deviceName[1024] = { 0 };
					UINT deviceNameLength = sizeof(deviceName) / sizeof(*deviceName);
					bool gotName = GetRawInputDeviceInfoW(rawInput->header.hDevice, RIDI_DEVICENAME, deviceName, &deviceNameLength) > 0;

					if (gotInfo && gotName)
					{
						if (IsDualshock4(deviceInfo.hid))
						{
							//UpdateDualshock4(rawInput->data.hid.bRawData, rawInput->data.hid.dwSizeHid, deviceName, &outputData);
						}
					}
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

		for (int32 controllerIndex = 0; controllerIndex < 4; controllerIndex++)
		{
			XINPUT_STATE controllerState = {};
			if (XInputGetState(controllerIndex, &controllerState) == ERROR_SUCCESS)
			{
				XINPUT_GAMEPAD* pad = &controllerState.Gamepad;

				input->controllerUp = pad->wButtons & XINPUT_GAMEPAD_DPAD_UP;
				input->controllerDown = pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN;
				input->controllerLeft = pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT;
				input->controllerRight = pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;
				input->controllerStart = pad->wButtons & XINPUT_GAMEPAD_START;
				input->controllerBack = pad->wButtons & XINPUT_GAMEPAD_BACK;
				input->controllerLeftThumb = pad->wButtons & XINPUT_GAMEPAD_LEFT_THUMB;
				input->controllerRightThumb = pad->wButtons & XINPUT_GAMEPAD_RIGHT_THUMB;
				input->controllerLeftShoulder = pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER;
				input->controllerRightShoulder = pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER;
				input->controllerA = pad->wButtons & XINPUT_GAMEPAD_A;
				input->controllerB = pad->wButtons & XINPUT_GAMEPAD_B;
				input->controllerX = pad->wButtons & XINPUT_GAMEPAD_X;
				input->controllerY = pad->wButtons & XINPUT_GAMEPAD_Y;

				input->controllerLeftTrigger = (real32)pad->bLeftTrigger / 255.0f;
				input->controllerRightTrigger = (real32)pad->bRightTrigger / 255.0f;
				input->controllerLeftThumbDrag = Vec2f((real32)pad->sThumbLX, (real32)pad->sThumbLY) / 32767.0f;
				input->controllerRightThumbDrag = Vec2f((real32)pad->sThumbRX, (real32)pad->sThumbRY) / 32767.0f;
			}
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