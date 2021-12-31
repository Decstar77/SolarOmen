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
		GetEditorState();

		es->camera.transform = Transform();
		es->camera.transform.position.y = 1;
		es->camera.yaw = 90.0f;
		es->camera.far_ = 250.0f;
		es->camera.near_ = 0.3f;
		es->camera.yfov = 45.0f;
		es->camera.aspect = (real32)ps->clientWidth / (real32)ps->clientHeight;

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

	template<typename T>
	bool ComboEnum(const CString& lable, int32* currentItem)
	{
		constexpr int32 count = (int32)T::Value::COUNT;
		const char* items[count] = {};
		for (int32 i = 0; i < count; i++)
		{
			items[i] = T::__STRINGS__[i].GetCStr();
		}

		return ImGui::Combo("Type", currentItem, items, count);
	}

	class TextFileWriter
	{
	public:
		TextFileWriter()
		{
			data = GameMemory::PushTransientStruct<LargeString<1000000>>();
		};

		template<typename T>
		inline void WriteLine(const T& t)
		{
			data->Add(t).Add('\n');
		}

		template<typename T>
		inline void Write(const T& t)
		{
			data->Add(t);
		}

		inline void SaveToDisk(const CString& path)
		{
			Platform::WriteFile(path, data->GetCStr(), (uint32)data->GetLength());
		}

	private:
		LargeString<1000000>* data;
	};

	static void SaveRoomAsset(RoomAsset* asset)
	{
		TextFileWriter file;

		file.WriteLine("Version: 1");
		file.WriteLine(CString("Player1 Start Pos:").Add(ToString(asset->player1StartPos)));
		file.WriteLine(CString("Player2 Start Pos:").Add(ToString(asset->player2StartPos)));
		file.WriteLine("Two Player Game: ?");
		file.WriteLine(CString("Entities:").Add(asset->entities.count));

		for (uint32 i = 0; i < asset->entities.count; i++)
		{
			EntityAsset* entity = &asset->entities[i];
			file.WriteLine("{");
			file.Write("Name: "); file.WriteLine(entity->name);
			file.Write("Tag: "); file.WriteLine(entity->tag.ToString());
			file.Write("Transform: "); file.WriteLine(entity->localTransform.ToString());

			if (entity->renderComponent.enabled)
			{
				GetAssetState();

				file.Write("Render: ");
				CString modelName = "NONE";
				CString textureName = "NONE";
				CString shaderName = "NONE";
				if (entity->renderComponent.modelId != INVALID_ASSET_ID)
					modelName = as->models.Get(entity->renderComponent.modelId)->name;
				if (entity->renderComponent.textureId != INVALID_ASSET_ID)
					textureName = as->textures.Get(entity->renderComponent.textureId)->name;
				if (entity->renderComponent.shaderId != INVALID_ASSET_ID)
					shaderName = as->shaders.Get(entity->renderComponent.shaderId)->name;

				file.Write(modelName); file.Write(':');
				file.Write(textureName); file.Write(':');
				file.Write(shaderName);
				file.WriteLine("");
			}

			if (entity->colliderComponent.enabled)
			{
				file.Write("Collider: ");
				file.Write((uint32)entity->colliderComponent.type); file.Write(':');
				switch (entity->colliderComponent.type)
				{
				case ColliderType::SPHERE: file.WriteLine(entity->colliderComponent.sphere.ToString()); break;
				case ColliderType::ALIGNED_BOUNDING_BOX: file.WriteLine(entity->colliderComponent.alignedBox.ToString()); break;
				default: Assert(0, "COLLIDER TYPE!!!");
				}
			}

			file.WriteLine("}");
		}

		file.WriteLine("Map:");
		for (uint32 i = 0; i < asset->map.GetCapcity(); i++)
		{
			int32 data = asset->map[i];
			file.Write(data);
			file.Write(" ");

			if (i % 25 == 0 && i != 0)
			{
				file.Write("\n");
			}
		}

		if (asset->name == "")
		{
			asset->name = "UnknownRoom";
		}

		file.SaveToDisk(CString("../Assets/Raw/Rooms/").Add(asset->name).Add(".txt"));
	}

	static void ShowAssetWindow()
	{
		GetEditorState();
		GetAssetState();
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
		ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
		ImGui::Begin("Assets", &es->showAssetWindow);

		ManagedArray<ModelAsset> models = as->models.GetValueSet();

		for (uint32 i = 0; i < models.count; i++)
		{
			ImGui::Text(models[i].name.GetCStr());
		}

		ImGui::End();
		ImGui::PopStyleVar();
	}

	static void ShowBuildWindow()
	{
		GetEditorState();
		GetAssetState();
		GetGameState();
		GetInput();

		for (uint32 i = 0; i < as->rooms.count; i++)
		{
			if (gs->currentRoom.name == as->rooms[i].name)
			{
				es->currentRoomAsset = as->rooms[i];
				break;
			}
		}

		if (gs->currentRoom.totalTime > 0)
		{
			gs->TransitionToRoom(es->currentRoomAsset);
			gs->currentRoom.isPaused = true;
		}

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
		ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
		ImGui::Begin("Build", &es->showBuildWindow);

		ImGui::DragFloat3("Player1 Start Position", es->currentRoomAsset.player1StartPos.ptr);
		ImGui::DragFloat3("Player2 Start Position", es->currentRoomAsset.player2StartPos.ptr);

		Room* room = &gs->currentRoom;

		es->currentRoomAsset.entities.Clear();
		room->BeginEntityLoop();
		while (Entity entity = room->GetNextEntity())
		{
			EntityAsset entityAsset = {};
			entityAsset.name = entity.GetName();
			entityAsset.tag = entity.GetTag();
			entityAsset.localTransform = entity.GetLocalTransform();
			entityAsset.renderComponent = *entity.GetRenderComponent();
			entityAsset.colliderComponent = entity.GetColliderLocal();

			es->currentRoomAsset.entities.Add(entityAsset);
		}

		static int32 currentItem = 0;
		ComboEnum<GridCellType>("Build type", &currentItem);

		Grid* grid = &room->grid;

		room->grid.DebugDraw();

		Ray ray = es->camera.ShootRayFromScreen();
		Plane plane = CreatePlane(0.0f, Vec3f(0, 1, 0));

		RaycastInfo info = {};
		if (RaycastPlane(ray, plane, &info))
		{
			ImGuiIO& io = ImGui::GetIO();
			if (!io.WantCaptureMouse)
			{
				GridCell* cell = room->grid.GetCellFromPosition(info.closePoint);
				if (input->mb1 && input->ctrl)
				{
					if (cell && cell->type != GridCellType::Value::EMPTY)
					{
						cell->type = GridCellType::Value::EMPTY;
						room->CreateEntitiesFromGripMap();
					}
				}
				else if (input->mb1)
				{
					if (cell && cell->type == GridCellType::Value::EMPTY)
					{
						cell->type = GridCellType::Value::WALL;
						room->CreateEntitiesFromGripMap();
					}
				}

			}
			//Debug::DrawPoint(info.closePoint);
		}

		if (IsKeyJustDown(input, s) && input->ctrl)
		{
			for (uint32 i = 0; i < grid->cells.GetCapcity(); i++)
			{
				es->currentRoomAsset.map[i] = (int32)grid->cells[i].type;
			}

			SaveRoomAsset(&es->currentRoomAsset);

			Debug::LogInfo(CString("Saved").Add(es->currentRoomAsset.name));
		}


		ImGui::End();
		ImGui::PopStyleVar();
	}


	template<typename T>
	AssetId ComboBoxOfAsset(const char* label, const ManagedArray<T>& assets, AssetId currentId)
	{
		int32 currentItem = 0;
		const char** items = GameMemory::PushTransientCount<const char*>(assets.count);
		CString* itemData = GameMemory::PushTransientCount<CString>(assets.count);

		for (uint32 i = 0; i < assets.count; i++)
		{
			if (assets[i].id == currentId) { currentItem = i; }
			itemData[i].Add(assets[i].name);
			items[i] = itemData[i].GetCStr();
		}

		if (ImGui::Combo(label, &currentItem, items, assets.count))
		{
			currentId = assets[currentItem].id;
		}

		return currentId;
	}

	static void DisplayTransform(Transform* transform, const real32 delta = 0.25f)
	{
		ImGui::PushStyleColor(ImGuiCol_FrameBg, (ImVec4)ImColor::HSV(0.0, 0.94f, 0.7f));
		ImGui::DragFloat3("Position", transform->position.ptr, delta);
		ImGui::PopStyleColor(1);

		ImGui::PushStyleColor(ImGuiCol_FrameBg, (ImVec4)ImColor::HSV(140.0f / 360.0f, 0.94f, 0.7f));
		Vec3f ori = QuatToEuler(transform->orientation);
		// @NOTE: if is for floating point accuracy
		if (ImGui::DragFloat3("Orientation", ori.ptr, delta))
		{
			transform->orientation = EulerToQuat(ori);
		}
		ImGui::PopStyleColor(1);

		ImGui::PushStyleColor(ImGuiCol_FrameBg, (ImVec4)ImColor::HSV(207.0f / 360.0f, 0.94f, 0.7f));
		ImGui::DragFloat3("Scale", transform->scale.ptr, delta, 0.1f, 1000.0f);
		ImGui::PopStyleColor(1);

		if (ImGui::Button("Reset transform"))
		{
			*transform = Transform();
		}
	}

	static void ShowInpsectorWindow()
	{
		GetEditorState();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
		ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
		ImGui::Begin("Inspector", &es->showInspectorWindow);

		if (es->selectedEntities.count > 0)
		{
			Entity entity = es->selectedEntities[0];
			if (entity.IsValid())
			{
				CString name = entity.GetName();
				if (ImGui::InputText("Name", name.GetCStr(), CString::CAPCITY)) { entity.SetName(name); }

				if (ImGui::CollapsingHeader("Local Transform"))
				{
					Transform transform = entity.GetLocalTransform();
					DisplayTransform(&transform);
					entity.SetLocalTransform(transform);
				}

				if (ImGui::CollapsingHeader("Rendering"))
				{
					GetAssetState();

					entity.SetModel(ComboBoxOfAsset("Model", as->models.GetValueSet(), entity.GetModelId()));
					entity.SetTexture(ComboBoxOfAsset("Texture", as->textures.GetValueSet(), entity.GetTextureId()));
					entity.SetShader(ComboBoxOfAsset("Shader", as->shaders.GetValueSet(), entity.GetShaderId()));
				}
			}
		}

		ImGui::End();
		ImGui::PopStyleVar();
	}

	static void DoEntityTreeDisplay(Entity entity)
	{
		ImGui::PushID(entity.GetId().index);
		if (ImGui::TreeNode(entity.GetName().GetCStr()))
		{
			//ImGui::Text(entity.GetId().ToString().GetCStr());
			//ImGui::SameLine();
			if (ImGui::SmallButton("Select"))
			{
				GetEditorState();
				es->selectedEntities.Clear();
				es->selectedEntities.Add(entity);
				es->showInspectorWindow = true;

				//	if (entity != es->selectedEntity)
				//	{
				//		UndoEntry entry = {};
				//		entry.action = UndoAction::Value::SELECTION_CHANGE;
				//		entry.selectionChange.current = entity->GetId();
				//		entry.selectionChange.previous = es->selectedEntity ? es->selectedEntity->GetId() : EntityId();

				//		es->selectedEntity = entity;
				//		es->undoSystem.Do(entry);
				//	}
			}

			//DisplayTransform(&entity->transform);

			for (Entity* child = entity.GetFirstChild(); child != nullptr; child = child->GetSiblingAhead())
			{
				DoEntityTreeDisplay(*child);
			}

			//Entity* parent = entity->GetFirstChild();
			//if (parent)
			//{
			//	DoDisplayEntity(es, parent);
			//}

			ImGui::TreePop();
		}
		ImGui::PopID();
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
				Entity* parent = entity.GetParent();
				if (!parent)
				{
					DoEntityTreeDisplay(entity);
				}
			}
		}

		ImGui::End();
		ImGui::PopStyleVar();
	}

	void OperateCamera(Camera* camera, real32 dtime)
	{
		GetInput();
		if (input->mb2)
		{
			input->mouse_locked = true;

			Vec2f delta = input->mouseDelta;
			delta.y = -delta.y;

			real32 mouse_sensitivity = 0.1f;

			real32 nyaw = camera->yaw - delta.x * mouse_sensitivity;
			real32 npitch = camera->pitch + delta.y * mouse_sensitivity;

			npitch = Clamp(npitch, -89.0f, 89.0f);
			camera->yaw = Lerp(camera->yaw, nyaw, 0.67f);
			camera->pitch = Lerp(camera->pitch, npitch, 0.67f);

			Vec3f direction;
			direction.x = Cos(DegToRad(camera->yaw)) * Cos(DegToRad(camera->pitch));
			direction.y = Sin(DegToRad(camera->pitch));
			direction.z = Sin(DegToRad(camera->yaw)) * Cos(DegToRad(camera->pitch));

			Transform transform = camera->transform;

			Mat4f result = LookAtLH(transform.position, transform.position + direction, Vec3f(0, 1, 0));

			camera->transform.orientation = Mat4ToQuat(result);

			Basisf basis = camera->transform.GetBasis();

			real32 move_speed = 6.0f;
			if (input->w)
			{
				camera->transform.position += (basis.forward * move_speed * dtime);
			}
			if (input->s)
			{
				camera->transform.position += (-1.0f * basis.forward * move_speed * dtime);
			}
			if (input->a)
			{
				camera->transform.position += (-1.0f * basis.right * move_speed * dtime);
			}
			if (input->d)
			{
				camera->transform.position += (basis.right * move_speed * dtime);
			}
			if (input->q)
			{
				camera->transform.position += (Vec3f(0, 1, 0) * move_speed * dtime);
			}
			if (input->e)
			{
				camera->transform.position += (Vec3f(0, -1, 0) * move_speed * dtime);
			}
		}
		else
		{
			input->mouse_locked = false;
		}
	}

	static void OperateMouseSelection()
	{
		GetInput();

		if (IsKeyJustDown(input, mb1))
		{
			GetEditorState();
			GetGameState();
			GetAssetState();

			Ray ray = es->camera.ShootRayFromScreen();
			Entity closestEntity = {};
			real32 closestDist = REAL_MAX;

			gs->currentRoom.BeginEntityLoop();
			while (Entity entity = gs->currentRoom.GetNextEntity())
			{
				if (entity.GetModelId() != INVALID_ASSET_ID)
				{
					Mat4f transform = entity.GetWorldTransform().CalculateTransformMatrix();

					UnpackedModelAsset model = as->models.Get(entity.GetModelId())->Unpack();
					for (uint32 i = 0; i < model.triangleCount; i += 1)
					{
						real32 dist = 0;
						// @SPEED: Transform ray instead 
						Triangle triangle = model.GetTriangle(i);
						triangle.v0 = Vec3f(Vec4f(triangle.v0, 1.0f) * transform);
						triangle.v1 = Vec3f(Vec4f(triangle.v1, 1.0f) * transform);
						triangle.v2 = Vec3f(Vec4f(triangle.v2, 1.0f) * transform);

						if (RaycastTriangle(ray, triangle, &dist))
						{
							if (dist < closestDist)
							{
								closestEntity = entity;
								closestDist = dist;
							}
						}
					}
				}
			}

			es->selectedEntities.Clear();
			if (closestEntity.IsValid())
			{
				es->showInspectorWindow = true;
				es->selectedEntities.Add(closestEntity);
			}
		}
	}

	void Editor::UpdateEditor(EntityRenderGroup* renderGroup, real32 dt)
	{
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		//ImGuizmo::BeginFrame();

		GetEditorState();
		GetInput();
		GetGameState();

		es->showConsoleWindow = 0;
		if (IsKeyJustDown(input, f5))
		{
			gs->currentRoom.isPaused = !gs->currentRoom.isPaused;
		}
		if (IsKeyJustDown(input, f6))
		{
			//gs->currentRoom.Initialize(false);
		}

		if (!gs->currentRoom.isPaused)
		{
			Game::UpdateGame(dt);
		}
		else
		{
			GetGameState();
			//gs->currentRoom.DEBUGDrawAllColliders();

			ShowMainMenuBar();
			if (es->showRoomWindow) ShowRoomWindow();
			if (es->showBuildWindow) ShowBuildWindow();
			if (es->showAssetWindow) ShowAssetWindow();
			if (es->showInspectorWindow) ShowInpsectorWindow();

			ImGuiIO& io = ImGui::GetIO();
			if (!io.WantCaptureMouse)
			{
				OperateCamera(&es->camera, dt);
				OperateMouseSelection();
			}

			renderGroup->playerCamera = es->camera;
		}

		if (es->showPerformanceWindow) ShowPerformanceWindow(dt);
		if (es->showConsoleWindow) ShowConsoleWindow();


	}

	void Editor::Shutdown()
	{
	}
}

