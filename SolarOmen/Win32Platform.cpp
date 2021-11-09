#include <fstream>
#include <filesystem>

#include "platform/SolarPlatform.h"
#include "Debug.h"
#include "SolarOmen.h"
#include "renderer/SolarRenderer.h"
#include "core/SolarMemory.h"

#include "editor/SolarEditor.h"

#include "serialization/assetId/ShaderId.h"

#ifndef FULL_WINTARD
#define WIN32_LEAN_AND_MEAN
#define NOGDICAPMASKS
//#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOSYSCOMMANDS
#define NORASTEROPS
#define OEMRESOURCE
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCTLMGR
#define NODRAWTEXT
#define NOKERNEL
#define NONLS
#define NOMEMMGR
#define NOMETAFILE
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX
#define NORPC
#define NOPROXYSTUB
#define NOIMAGE
#define NOTAPE
#define NOMINMAX
#endif

#include <shobjidl.h>
#include <Windows.h>
#include "Editor.h"

//#define WINDOW_WIDTH 1920
//#define WINDOW_HEIGHT 1080
#define WINDOW_WIDTH 1900
#define WINDOW_HEIGHT 1000
//#define WINDOW_WIDTH 1280
//#define WINDOW_HEIGHT 720
#define TITLE "Hello" 

static bool game_running = true;

namespace cm
{
	inline uint32 SafeTruncateUInt64(uint64 Value)
	{
		// TODO: Defines for maximum values
		Assert(Value <= 0xFFFFFFFF, "Trunc err");
		uint32 Result = (uint32)Value;
		return(Result);
	}

	int32 PlatformCompareFileTimes(uint64 fileTime1, uint64 fileTime2)
	{
		ULARGE_INTEGER large = {};

		FILETIME f1 = {};
		large.QuadPart = fileTime1;
		f1.dwLowDateTime = large.LowPart;
		f1.dwHighDateTime = large.HighPart;

		FILETIME f2 = {};
		large.QuadPart = fileTime2;
		f2.dwLowDateTime = large.LowPart;
		f2.dwHighDateTime = large.HighPart;

		return (int32)CompareFileTime(&f1, &f2);
	}

	CString PlatformOpenNFileDialogAndReturnPath()
	{
		char output[256] = {};

		HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
			COINIT_DISABLE_OLE1DDE);
		if (SUCCEEDED(hr))
		{
			IFileOpenDialog* pFileOpen;

			// Create the FileOpenDialog object.
			hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
				IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

			if (SUCCEEDED(hr))
			{
				// Show the Open dialog box.
				hr = pFileOpen->Show(NULL);

				// Get the file name from the dialog box.
				if (SUCCEEDED(hr))
				{
					IShellItem* pItem;
					hr = pFileOpen->GetResult(&pItem);
					if (SUCCEEDED(hr))
					{
						PWSTR pszFilePath;
						hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

						// Display the file name to the user.
						if (SUCCEEDED(hr))
						{
							sprintf_s(output, "%ws", pszFilePath);
							CoTaskMemFree(pszFilePath);
						}
						pItem->Release();
					}
				}
				pFileOpen->Release();
			}
			CoUninitialize();
		}

		return CString(output);
	}

	bool32 DEBUGWriteFile(PlatformFile file, const CString& name)
	{
		bool32 result = false;

		HANDLE FileHandle = CreateFileA(name.GetCStr(), GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
		if (FileHandle != INVALID_HANDLE_VALUE)
		{
			DWORD BytesWritten;

			if (WriteFile(FileHandle, file.data, SafeTruncateUint64(file.size_bytes), &BytesWritten, 0))
			{
				// NOTE(casey): File read successfully
				result = (BytesWritten == file.size_bytes);
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

	PlatformFile DEBUGLoadEntireFile(const CString& file, bool32 metaDataOnly)
	{
		PlatformFile result = {};
		result.path = file;

		HANDLE FileHandle = CreateFileA(file.GetCStr(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
		if (FileHandle != INVALID_HANDLE_VALUE)
		{
			LARGE_INTEGER FileSize;
			if (GetFileSizeEx(FileHandle, &FileSize))
			{
				uint32 FileSize32 = SafeTruncateUInt64(FileSize.QuadPart);
				result.data = malloc(FileSize32);
				if (result.data)
				{
					if (!metaDataOnly)
					{
						DWORD BytesRead;

						if (ReadFile(FileHandle, result.data, FileSize32, &BytesRead, 0) &&
							(FileSize32 == BytesRead))
						{
							result.size_bytes = FileSize32;
						}
						else
						{
							// TODO: Logging
							DEBUGFreeFile(&result);
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

	static std::vector<CString> DEBUGGetFileDirectories(const CString& path, const CString& fileTypes)
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
						std::vector<CString> subFolder = DEBUGGetFileDirectories(filePath.Add('/'), fileTypes);
						for (const CString& sub : subFolder)
						{
							result.push_back(sub);
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

		return result;
	}

	PlatformFolder DEBUGLoadEnitreFolder(const CString& path, const CString& fileTypes, bool32 metaDataOnly)
	{
		PlatformFolder result = {};
		result.path = path;

		std::vector<CString> files = DEBUGGetFileDirectories(path, fileTypes);

		result.files.resize(files.size());
		for (int32 fileIndex = 0; fileIndex < (int32)files.size(); fileIndex++)
		{
			result.files[fileIndex] = DEBUGLoadEntireFile(files.at(fileIndex), metaDataOnly);
		}

		return result;
	}

	void DEBUGFreeFolder(PlatformFolder* folder)
	{
		if (folder)
		{
			for (int32 fileIndex = 0; fileIndex < (int32)folder->files.size(); fileIndex++)
			{
				DEBUGFreeFile(&folder->files[fileIndex]);
			}

			folder->files.clear();
		}
	}

	void DEBUGFreeFile(PlatformFile* file)
	{
		if (file->data)
		{
			free(file->data);
			file->data = nullptr;
		}
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

	void StringTests(TransientState* ts)
	{
		CString s1 = "    H   e    l   l   o  worl d     ";
		CString s2 = "Hello world";
		CString s3 = "Hello world";
		CString s4 = "Hel";
		CString s5 = " Hel  ";
		CString s6 = "98374";
		CString s7 = "../Assets/Processed/Worlds/Palette.txt";

		if (s1 == s2)
		{
			int a = 2;
		}
		if (s2 == s3)
		{
			int a = 2;
		}

		s3.Replace('l', 'X');

		std::vector<CString> someSplit = s1.Split(' ');

		s1.RemoveWhiteSpace();
		s5.RemoveWhiteSpace();
		if (s1.StartsWith("Hell"))
		{
			int a = 2;
		}
		if (s1.StartsWith("Hellw"))
		{
			int a = 2;
		}
		if (s4.StartsWith("He"))
		{
			int a = 2;
		}

		s5 = s7;


		s7.Clear();


		CString s8;
		s8.Add("Hello");
		s8.Add(" Age");
		//s8.Add(':');
		//s8.Add(1002);
		//s8.Add(':');
		//s8.Add(3.145423f);

		int32 tes = s8.ToInt32();

		PlatformFile file = DEBUGLoadEntireFile(s5.GetCStr(), false);


		LargeString<100>* largeString = GameMemory::PushTransientStruct<LargeString<100>>();


		DEBUGFreeFile(&file);
		//int some = s6.ToInt32();
	}

	const char* t1 = "Something1";
	const char* t2 = "Something2";

	WORK_CALLBACK(TestWWorkCallback)
	{
		LOG((const char*)data);
	}

	WORK_CALLBACK(WorkCallbackArrays)
	{
		LOG("Begin");

		ManagedArray<real32> reals = GameMemory::GetManagedArray<real32>();
		ManagedArray<int32> ints = GameMemory::GetManagedArray<int32>();
		ManagedArray<bool> bools = GameMemory::GetManagedArray<bool>();

		reals.Add(0.3f);

		GameMemory::Release(&reals);
		GameMemory::Release(&ints);
		GameMemory::Release(&bools);

		LOG("End");
	}

	Mat3f CalcBoxInertiaTensor(real32 w, real32 h, real32 d, real32 m, Vec3f cm)
	{
		Mat3f tensor = Mat3f(0);
		tensor[0][0] = (h * h + d * d) / 12.0f;
		tensor[1][1] = (w * w + d * d) / 12.0f;
		tensor[2][2] = (w * w + h * h) / 12.0f;

		Vec3f R = Vec3f(0, 0, 0) - cm;
		real32 R2 = MagSqrd(R);

		Mat3f patTensor = Mat3f(0);
		patTensor[0] = Vec3f(R2 - R.x * R.x, R.x * R.y, R.x * R.z);
		patTensor[1] = Vec3f(R.y * R.x, R2 - R.y * R.y, R.y * R.z);
		patTensor[2] = Vec3f(R.z * R.x, R.z * R.y, R2 - R.z * R.z);

		tensor = (tensor + patTensor);

		return tensor;
	}

	void RunTests()
	{
		// TestSignedVolumeProjection();
		//ProgramMemory* pmem = (ProgramMemory*)malloc(sizeof(ProgramMemory));
		//ZeroStruct(pmem);
		//pmem->instance = pmem;

		//ManagedArray<int32> arr = ProgramMemory::GetManagedArray<int32>();

		//arr.Add(20);
		//int v = arr[0];

		//free(pmem);

		//Platform::AddWorkEntry(Platform::WorkEntry(TestWWorkCallback, (void*)t1));
		//Platform::AddWorkEntry(Platform::WorkEntry(TestWWorkCallback, (void*)t2));

#if 1
		CString s1 = "Hello I'm Slow";

		bool32 b1 = s1.Contains("I'm ");
		bool32 b2 = s1.Contains("Slow");
		bool32 b3 = s1.Contains("Hell");

		bool32 b4 = s1.Contains("Hellw");
		bool32 b5 = s1.Contains("Well");
#endif
#if 0
		Vec3f t1 = Vec3f(0, 1, 0);
		Vec3f t2 = Vec3f(0, 0, 1);
		Vec3f t3 = Vec3f(0.24, 93.4, 1);
		Vec3f t4 = Vec3f(23.0f, 490.0f, -129.0f);
		Vec3f t5 = Vec3f(-1.4f, 25.0, 4.05f);

		LOGTS(Cross(t1, t2));
		LOGTS(Cross(t3, t2));
		LOGTS(Cross(t2, t2));
		LOGTS(Cross(t5, t4));
		LOGTS(Cross(t1, t3));
#endif
#if 0
		Quatf q1(0.238, 043, -2.40, 1.32);

		q1 = Normalize(q1);
		LOGTS(q1);

		LOGTS(Mat3f(1));
		LOGTS(QuatToMat3(q1));
#endif
#if 0
		Quatf q1(0.238, 043, -2.40, 1.32);
		Quatf q2(0.938, 0.33, 2.40, -1.02);
		Quatf q3(1.28, 89.163, 0.158, 1.32);

		LOGTS(q1 * q2);

		//Mat3f t0;
		//t0.row0 = RotatePointLHS(q1, t0.row0);
		//t0.row1 = RotatePointLHS(q1, t0.row1);
		//t0.row2 = RotatePointLHS(q1, t0.row2);

		Mat3f m1 = QuatToMat3(Normalize(q1));
		Mat3f m2 = QuatToMat3(Normalize(q2));
		Mat3f m3 = QuatToMat3(Normalize(q3));

		Mat3f m4 = (m1)*m2 * (m1);

		LOGTS(m1);
		LOGTS(m2);
		LOGTS(m3);
		LOGTS(m4);
#endif
#if 0
		Mat3f I1 = CalcBoxInertiaTensor(1.0, 1.0, 1.0, 1.0, Vec3f(0.5, 0, 0));
		Mat3f I2 = CalcBoxInertiaTensor(1.0, 1.0, 1.0, 1.0, Vec3f(-0.5, 0, 0));
		Mat3f IT = CalcBoxInertiaTensor(2.0, 1.0, 1.0, 2.0, Vec3f(0, 0, 0));
		LOGTS(I1);
		LOGTS(I2);
		LOGTS(1.0f * (I1 + I2));
		LOGTS(2.0f * (IT));
#endif
#if 0
		ManagedArray<real32> reals = GameMemory::GetManagedArray<real32>();
		ManagedArray<int32> ints = GameMemory::GetManagedArray<int32>();
		ManagedArray<bool> bools = GameMemory::GetManagedArray<bool>();

		reals.Add(0.3f);

		GameMemory::Release(&reals);
		GameMemory::Release(&ints);
		GameMemory::Release(&bools);
#endif
#if 0
		Platform::WorkEntry work(WorkCallbackArrays, nullptr);
		Platform::AddWorkEntry(work);
		Platform::AddWorkEntry(work);
		Platform::AddWorkEntry(work);
		Platform::AddWorkEntry(work);
		Platform::AddWorkEntry(work);

		bool hasWork = Platform::HasWork();
		Platform::WaitForAllWorkToComplete();
		hasWork = Platform::HasWork();

		ManagedArray<real32> reals = GameMemory::GetManagedArray<real32>();
#endif
		int a = 2;
	}

	struct GameWork
	{
		GameState* gs;
		AssetState* as;
		PlatformState* ws;
		Input* input;
		EntityRenderGroup* renderGroup;
	};

	WORK_CALLBACK(GameWorkCallback)
	{
		GameWork* gameWork = (GameWork*)data;
		UpdateGame(gameWork->gs, gameWork->as, gameWork->ws, gameWork->input);
		ConstructRenderGroup(gameWork->gs, gameWork->renderGroup);
	}


}




using namespace cm;
LRESULT CALLBACK WindProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
static void Win32ProcessPendingMessages(PlatformState* win_state, Input* input);

//int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
int main()
{
	AllocConsole();
	AttachConsole(GetCurrentProcessId());
	FILE* stream;
	freopen_s(&stream, "CONOUT$", "w+", stdout);
	freopen_s(&stream, "CONOUT$", "w+", stderr);

	HWND console = GetConsoleWindow();
	SetWindowPos(console, 0, 1920 / 2 + 1000, 1080 / 2 - 250, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	RECT client_rect;
	client_rect.left = 0;
	client_rect.right = (LONG)WINDOW_WIDTH;
	client_rect.top = 0;
	client_rect.bottom = (LONG)WINDOW_HEIGHT;

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
	window_class.lpszClassName = TITLE;

	RegisterClassExA(&window_class);

	HWND window = CreateWindowExA(
		NULL, window_class.lpszClassName, TITLE, window_style,
		CW_USEDEFAULT, CW_USEDEFAULT, client_rect.right - client_rect.left, client_rect.bottom - client_rect.top,
		NULL, NULL, NULL, NULL);

	GetClientRect(window, &client_rect);

	PlatformState win_state = {};
	win_state.window = (int64)window;
	win_state.console = (int64)console;
	win_state.client_width = client_rect.right - client_rect.left;
	win_state.client_height = client_rect.bottom - client_rect.top;
	win_state.aspect = (real32)win_state.client_width / (real32)win_state.client_height;

	Platform::CreateThreads();
	Platform::CreateNetworking();


	if (window)
	{
		// NOTE: Adjust window pos ect
		int32 window_pos_x = GetSystemMetrics(SM_CXSCREEN) / 2 - win_state.client_width / 2;
		int32 window_pos_y = GetSystemMetrics(SM_CYSCREEN) / 2 - win_state.client_height / 2;
		SetWindowPos(window, NULL, window_pos_x, window_pos_y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		//================================================================//

		// NOTE: Initialize raw input --- duh
		InitializeRawInput(&win_state);
		//================================================================//

		// @NOTE: Allocate all the memory for the game
		GameMemory::AllocateGameMemory(Gigabytes(1), Gigabytes(2));
		//================================================================//

		// @NOTE: Set up input structs
		Input old_input = {};
		Input new_input = {};
		new_input.old_input = &old_input;
		//================================================================//

		// @NOTE: Allocate all the states
		GameState* gameState = GameMemory::PushPermanentStruct<GameState>();
		RenderState* renderState = GameMemory::PushPermanentStruct<RenderState>();
		AssetState* assetState = GameMemory::PushPermanentStruct<AssetState>();
		TransientState* transientState = GameMemory::PushTransientStruct<TransientState>();
#if EDITOR
		EditorState* editorState = GameMemory::PushPermanentStruct<EditorState>();
		RunTests();
#endif

		//================================================================//

		// @NOTE: Load every asset in the game
		InitializeAssets(assetState);
		//================================================================//

		assetState->shaderCount = 1;

		InitializeDebugState();
		InitializeRenderState(renderState, assetState, &win_state);
		InitializeGameState(gameState, assetState, &win_state);

#if EDITOR
		InitializeEditorState(gameState, renderState, assetState, editorState, &win_state);
#endif

		EntityRenderGroup* renderGroupA = GameMemory::PushPermanentStruct<EntityRenderGroup>();
		EntityRenderGroup* renderGroupB = GameMemory::PushPermanentStruct<EntityRenderGroup>();

		EntityRenderGroup* renderGroupGame = renderGroupA;
		EntityRenderGroup* renderGroupRender = renderGroupB;

		UpdateGame(gameState, assetState, &win_state, &new_input);
		ConstructRenderGroup(gameState, renderGroupGame);

		gameState->dt = 0.016f;
		while (game_running)
		{
			Clock clock;
			clock.Start();

			// @NOTE: Update the input
			// @NOTE: We don't clear the new input as windows is constantly updating everything 
			old_input = new_input;
			Win32ProcessPendingMessages(&win_state, &new_input);

			if (new_input.escape)
			{
				game_running = false;
			}

			// @NOTE: 
			ZeroStruct(transientState);
#if EDITOR
			renderGroupGame->ClearRenderGroup();
			UpdateEditor(gameState, renderState, assetState, editorState, &new_input, &win_state, renderGroupGame);
			RenderGame(renderState, assetState, renderGroupGame, &win_state, &new_input);
			RenderEditor(editorState);
			PresentFrame(renderState, editorState->vsync);
#else
			// @NOTE: Swap the threaded render groups

			Swap(renderGroupGame, renderGroupRender);
			renderGroupGame->ClearRenderGroup();

			GameWork gameWork = { };
			gameWork.gs = gameState;
			gameWork.as = assetState;
			gameWork.input = &new_input;
			gameWork.ws = &win_state;
			gameWork.renderGroup = renderGroupGame;

			Platform::WorkEntry editorGameWorkEntry = Platform::WorkEntry(GameWorkCallback, &gameWork);
			Platform::AddWorkEntry(editorGameWorkEntry);

			RenderGame(renderState, assetState, renderGroupRender, &win_state, &new_input);
			PresentFrame(renderState, true);

			Platform::WaitForAllWorkToComplete();
#endif

			GameMemory::ReleaseAllTransientMemory();

			clock.End();
			gameState->dt = clock.Get().delta_seconds;
		}

		ShutdownRenderState(renderState);
	}

	Platform::ShudownNewtorking();
	Platform::ShutdownThreads();

	return 0;
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

static void Win32ProcessPendingMessages(PlatformState* win_state, Input* input)
{
	win_state->isFocused = (bool32)GetFocus();
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

		mx = Clamp<real32>(mx, 0.0f, (real32)win_state->client_width);
		my = Clamp<real32>(my, 0.0f, (real32)win_state->client_height);

		input->mouse_pixl.x = mx;
		input->mouse_pixl.y = my;

		if (input->mouse_locked && win_state->isFocused)
		{
			SetCursor(FALSE);

			input->old_input->mouse_pixl = Vec2f((real32)(win_state->client_width / 2),
				(real32)(win_state->client_height / 2));

			POINT p = {};
			p.x = win_state->client_width / 2;
			p.y = win_state->client_height / 2;

			ClientToScreen((HWND)win_state->window, &p);

			SetCursorPos(p.x, p.y);
		}

		input->del = (GetKeyState(VK_DELETE) & (1 << 15));

		input->mouse_norm.x = mx / (real32)win_state->client_width;
		input->mouse_norm.y = my / (real32)win_state->client_height;

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

		mx = Clamp<real32>(mx, 0.0f, (real32)win_state->client_width);
		my = Clamp<real32>(my, 0.0f, (real32)win_state->client_height);

		input->mouse_pixl.x = mx;
		input->mouse_pixl.y = my;

		if (input->mouse_locked && win_state->isFocused)
		{
			SetCursor(FALSE);

			input->old_input->mouse_pixl = Vec2f((real32)(win_state->client_width / 2),
				(real32)(win_state->client_height / 2));

			POINT p = {};
			p.x = win_state->client_width / 2;
			p.y = win_state->client_height / 2;

			ClientToScreen((HWND)win_state->window, &p);

			SetCursorPos(p.x, p.y);
		}

		input->mouse_norm.x = mx / (real32)win_state->client_width;
		input->mouse_norm.y = my / (real32)win_state->client_height;

		input->shift = (GetKeyState(VK_SHIFT) & (1 << 15));
		input->alt = (GetKeyState(VK_MENU) & (1 << 15));
		input->ctrl = (GetKeyState(VK_CONTROL) & (1 << 15));

		input->mb1 = GetKeyState(VK_LBUTTON) & (1 << 15);
		input->mb2 = GetKeyState(VK_RBUTTON) & (1 << 15);
		input->mb3 = GetKeyState(VK_MBUTTON) & (1 << 15);

		input->mouseDelta = input->mouse_pixl - input->old_input->mouse_pixl;
	}

	if (!win_state->isFocused)
	{
		Input* oldInput = input->old_input;
		ZeroStruct(input);
		input->old_input = oldInput;
	}
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
		game_running = false;
		PostQuitMessage(0);
	}
	break;

	case WM_CLOSE:
	{
		game_running = false;
		PostQuitMessage(0);
	}
	break;

	case WM_MOUSEMOVE:
	{
		//real32 x = LOWORD(lparam);
		//real32 y = HIWORD(lparam);

		//MouseInput::SetLastPosition(MouseInput::GetCurrentPosition());
		//MouseInput::SetCurrentPosition(Vec2f(static_cast<real32>(x), static_cast<real32>(y)));
		//MouseInput::SetMouseDelta(MouseInput::GetCurrentPosition() - MouseInput::GetLastPosition());

		//POINT center_point = {};
		//if (ClientToScreen(window, &center_point))
		//{
		//	center_point.x += static_cast<LONG>(client_dimensions.x) / 2;
		//	center_point.y += static_cast<LONG>(client_dimensions.y) / 2;

		//	SetCursorPos(center_point.x, center_point.y);
		//}

		//InputCallbacks::PostMouseCallbacks();
	}
	break;

	default:
	{
		result = DefWindowProcA(hwnd, msg, wparam, lparam);
	}
	}

	return result;
}

