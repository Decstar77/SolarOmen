
#include <SolarEngine.h>
#include <src/renderer/SolarRenderer.h>
#include "src/SolarEntry.h"

#include "../vendor/imgui/imgui.h"
#include "../vendor/imgui/imgui_impl_win32.h"
#include "../vendor/imgui/imgui_impl_dx11.h"

#include <Windows.h>

#include "../vendor/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace sol
{
	struct GameState
	{

	};

	static bool8 ImguiWin32MessageCallback(uint16 eventCode, void* sender, void* listener, EventContext data)
	{
		struct Win32EventPumpMessageContext
		{
			HWND hwnd;
			UINT msg;
			WPARAM wparam;
			LPARAM lparam;
		};

		Win32EventPumpMessageContext* cxt = (Win32EventPumpMessageContext*)&data;

		LRESULT result = ImGui_ImplWin32_WndProcHandler(cxt->hwnd, cxt->msg, cxt->wparam, cxt->lparam);
		if (result)
		{
			*(LRESULT*)sender = result;
			return true;
		}

		return false;
	}

	static bool8 GameInitialze(Game* game)
	{
		ImGui::CreateContext();
		//ImNodes::CreateContext();
		ImGui::StyleColorsDark();

		if (ImGui_ImplWin32_Init(Platform::GetNativeState()))
		{
			struct DC
			{
				ID3D11Device* device;
				ID3D11DeviceContext* context;
			};

			DC* dc = (DC*)Renderer::GetNativeDeviceContext();
			if (ImGui_ImplDX11_Init(dc->device, dc->context))
			{
				if (EventSystem::Register((uint16)EventCodeEngine::WINDOW_PUMP_MESSAGES, nullptr, ImguiWin32MessageCallback))
				{
					return true;
				}
				else
				{

				}
			}
			else
			{
				SOLFATAL("Could not start dx11 imgui -> make sure the deviceContext is the first thing in the struct");
			}
		}
		else
		{
			SOLFATAL("Could not start win32 imgui -> make sure the window handle is the first thing in the struct");
		}


		return false;
	}

	static bool8 GameUpdate(Game* game, real32 dt)
	{
		Input* input = Input::Get();
		if (IsKeyJustDown(input, escape))
		{
			return 0;
		}

		return 1;
	}

	static bool8 GameRender(Game* game, real32 dt)
	{
		return 1;
	}

	static void GameOnResize(Game* game, uint32 width, uint32 height)
	{

	}

	bool8 CreateGame(Game* game)
	{
		game->appConfig.startPosX = 100;
		game->appConfig.startPosY = 100;
		game->appConfig.startWidth = 1280;
		game->appConfig.startHeight = 720;
		game->appConfig.name = "Engine Editor";
		game->Initialize = GameInitialze;
		game->Update = GameUpdate;
		game->Render = GameRender;
		game->OnResize = GameOnResize;

		game->gameState = GameMemory::PushPermanentStruct<GameState>();

		return true;
	}

}