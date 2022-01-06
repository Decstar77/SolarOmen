#include "ImGuiLayer.h"
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

	static bool8 ImguiDrawMessageCallback(uint16 eventCode, void* sender, void* listener, EventContext data)
	{
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		return false;
	}

	bool8 InitialzieImGui()
	{
		if (ImGui::CreateContext())
		{
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
						if (EventSystem::Register((uint16)EventCodeEngine::ON_RENDER_END, nullptr, ImguiDrawMessageCallback))
						{
							return true;
						}
						else
						{

						}
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
		}

		return false;
	}

	void UpdateImGui()
	{
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();

		ImGui::NewFrame();
		ImGui::ShowDemoWindow();
	}
}

