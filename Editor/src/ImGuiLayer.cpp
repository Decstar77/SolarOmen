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

	static void ShowMainMenuBar(EditorState* es)
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New")) {}
				if (ImGui::MenuItem("Open", "Ctrl+O")) {
					//CString path = PlatformOpenNFileDialogAndReturnPath();
					//LoadAGameWorld(gs, rs, as, es, path);
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("View"))
			{
				if (ImGui::MenuItem("Room Window")) { es->showRoomWindow = true; }
				if (ImGui::MenuItem("Render Settings")) { es->showRenderSettingsWindow = true; }
				if (ImGui::MenuItem("Performance")) { es->showPerformanceWindow = true; }
				if (ImGui::MenuItem("Console")) { es->showConsoleWindow = true; }
				if (ImGui::MenuItem("Build")) { es->showBuildWindow = true; }
				if (ImGui::MenuItem("Inspector")) { es->showInspectorWindow = true; }
				if (ImGui::MenuItem("Asset")) { es->showAssetWindow = true; }
				//if (ImGui::MenuItem("Main window")) { es->windowOpen = true; }
				//if (ImGui::MenuItem("Physics")) { es->physicsWindowOpen = true; }
				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}
	}

	static void ShowPerformanceWindow(EditorState* es, real32 dt)
	{
		for (int32 i = 0; i < ArrayCount(es->frameTimes) - 1; i++)
		{
			es->frameTimes[i] = es->frameTimes[i + 1];
		}
		es->frameTimes[ArrayCount(es->frameTimes) - 1] = dt * 1000;

		es->minTime = Min(es->minTime, dt);
		es->maxTime = Max(es->maxTime, dt);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
		ImGui::Begin("Performance", &es->showPerformanceWindow);
		ImGui::Text("Min %f", es->minTime * 1000.0f);
		ImGui::Text("max %f", es->maxTime * 1000.0f);
		if (ImGui::Button("Reset"))
		{
			es->minTime = REAL_MAX;
			es->maxTime = REAL_MIN;
		}
		ImGui::PlotLines("Frame time", es->frameTimes, ArrayCount(es->frameTimes), 0, 0, 0, 30, ImVec2(128, 128), 4);

		ImGui::End();
		ImGui::PopStyleVar();
	}

	void UpdateImGui(EditorState* es, real32 dt)
	{
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();

		ImGui::NewFrame();
		//ImGui::ShowDemoWindow();

		ShowMainMenuBar(es);
		if (es->showPerformanceWindow) { ShowPerformanceWindow(es, dt); }
	}
}

