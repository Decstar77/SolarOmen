#include "GameEditor.h"
#include "core/SolarEditor.h"

#include "core/SolarGame.h"
#include "game/TankGame.h"
#include "renderer/dx11/DX11Renderer.h"

#include "../vendor/imgui/imgui.h"
#include "../vendor/imgui/imgui_impl_win32.h"
#include "../vendor/imgui/imgui_impl_dx11.h"
#include "../vendor/imguizmo/ImGuizmo.h"
#include "../vendor/imgui_node/imnodes.h"

namespace cm
{
	bool32 Editor::Initialize()
	{
		ImGui::CreateContext();
		//ImNodes::CreateContext();
		ImGui::StyleColorsDark();

		GetPlatofrmState();
		GetRenderState();

		ImGui_ImplWin32_Init((void*)ps->window);
		ImGui_ImplDX11_Init(rs->device, rs->context);

		EditorState::Initialize(GameMemory::PushPermanentStruct<EditorState>());

		return true;
	}

	void Editor::RenderEditor()
	{
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}

	static void ShowMainMenuBar()
	{
		GetEditorState();

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
				//if (ImGui::MenuItem("Main window")) { es->windowOpen = true; }
				//if (ImGui::MenuItem("Physics")) { es->physicsWindowOpen = true; }
				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}
	}

	static void ShowPerformanceWindow(real32 dt) {
		GetEditorState();

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

	static void ShowConsoleWindow()
	{
		GetEditorState();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
		ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
		ImGui::Begin("Console", &es->showConsoleWindow);

		const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
		ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar);

		GetDebugState();

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
		for (uint32 i = 0; i < ds->logs.count; i++)
		{
			const char* item = ds->logs[i].GetCStr();
			//if (!Filter.PassFilter(item))
			//	continue;
			ImGui::TextUnformatted(item);
		}
		ImGui::PopStyleVar();
		ImGui::EndChild();
		ImGui::Separator();

		char inputBuf[256] = {};
		if (ImGui::InputText("Input", inputBuf, ArrayCount(inputBuf), ImGuiInputTextFlags_EnterReturnsTrue))
		{
			Debug::ExecuteCommand(inputBuf);
		}

		ImGui::End();
		ImGui::PopStyleVar();
	}

	static void ShowRoomWindow()
	{
		GetGameState();
		GetEditorState();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
		ImGui::Begin("Room", &es->showRoomWindow);

		if (ImGui::CollapsingHeader("Entity tree"))
		{
			Room* room = &gs->currentRoom;
			room->BeginEntityLoop();
			while (Entity entity = room->GetNextEntity())
			{
				ImGui::Text(entity.GetName().GetCStr());
			}
		}

		ImGui::End();
		ImGui::PopStyleVar();
	}

	void Editor::UpdateEditor(EntityRenderGroup* renderGroup, real32 dt)
	{
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		//ImGuizmo::BeginFrame();

		GetEditorState();

		ShowMainMenuBar();
		if (es->showConsoleWindow) ShowConsoleWindow();
		if (es->showPerformanceWindow) ShowPerformanceWindow(dt);
		if (es->showRoomWindow) ShowRoomWindow();

		GetInput();
		static bool inGame = false;
		if (IsKeyJustDown(input, f5))
		{
			inGame = !inGame;
		}


		if (inGame)
		{
			Game::UpdateGame(dt);
		}
		else
		{
			//ImGui::ShowDemoWindow();
			//ShowPerformanceWindow(dt);

		}
	}

	void Editor::Shutdown()
	{
	}
}

