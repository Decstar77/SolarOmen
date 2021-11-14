#include "SolarEditor.h"

namespace cm
{
	void InitializeEditorState(GameState* gs, RenderState* rs, AssetState* as, EditorState* es, PlatformState* ws)
	{
		ImGui::CreateContext();
		ImNodes::CreateContext();
		ImGui::StyleColorsDark();

		ImVec4* colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_Header] = ImVec4(1.00f, 1.00f, 1.00f, 0.31f);

		ImGui_ImplWin32_Init((void*)ws->window);
		ImGui_ImplDX11_Init(rs->device, rs->context);

		es->camera.transform = Transform();
		es->camera.transform.position.y = 1;
		es->camera.yaw = 90.0f;
		es->camera.far_ = 250.0f;
		es->camera.near_ = 0.3f;
		es->camera.yfov = 45.0f;
		es->camera.aspect = (real32)ws->client_width / (real32)ws->client_height;

		es->vsync = true;
		es->inGame = false;
		//es->currentGameFile = {};

		es->gizmo.mode = GizmoMode::Value::WORLD;
		es->gizmo.operation = GizmoOperation::Value::TRANSLATE;

		es->currentWorld = WorldId::Value::DEMO;

		es->nodeWindow.Initialize();
	}

	inline static void DrawEditorFrame()
	{
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}

	void RenderEditor(EditorState* es)
	{
		DrawEditorFrame();
	}

	void BeginEditorNewFrame()
	{
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();
	}

	void OperateCamera(EditorState* es, Input* input, real32 dtime)
	{
		input->mouse_locked = true;

		Camera* camera = &es->camera;

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

	static void DoDisplayEntity(EditorState* es, Entity* entity)
	{
		ImGui::PushID(entity->GetId().index);
		if (ImGui::TreeNode(entity->name.GetCStr()))
		{
			ImGui::Text(entity->GetId().ToString().GetCStr());
			ImGui::SameLine();
			if (ImGui::SmallButton("Select"))
			{
				es->selectedEntity = entity;
			}

			//DisplayTransform(&entity->transform);

			for (Entity* child = entity->GetFirstChild(); child != nullptr; child = entity->GetSibling())
			{
				DoDisplayEntity(es, child);
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

	static void DisplayWorldWindow(EditorState* es, GameState* gs)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
		ImGui::SetNextWindowPos({ 0,18 });
		ImGui::SetNextWindowSize({ 376, 731 });
		ImGui::Begin("World name", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

		gs->BeginEntityLoop();
		while (Entity* entity = gs->GetNextEntity())
		{
			Entity* parent = entity->GetParent();
			if (!parent)
			{
				DoDisplayEntity(es, entity);
			}
		}

		ImGui::End();
		ImGui::PopStyleVar();
	}

	template<typename T>
	T DisplayFancyEnum(const char* name, const T& t)
	{
		uint32 count = (uint32)T::Value::COUNT;
		Array<T> values = GameMemory::PushTransientArray<T>(count);
		Array<CString> strings = GameMemory::PushTransientArray<CString>(count);
		Array<const char*> items = GameMemory::PushTransientArray<const char*>(count);
		int32 currentItem = -1;
		for (uint32 i = 0; i < count; i++)
		{
			values[i] = T::ValueOf(i);
			strings[i] = values[i].ToString();
			items[i] = strings[i].GetCStr();
			if (t == values[i])
			{
				currentItem = i;
			}
		}

		Assert(currentItem != -1, "Fancy enum broke !");

		if (ImGui::Combo(name, &currentItem, items.data, count))
		{
			return T::ValueOf(currentItem);
		}

		return t;
	}

	static void DisplayEntityInspector(Entity* entity)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);

		ImGui::SetNextWindowPos({ 1900 - 376, 18 });
		ImGui::SetNextWindowSize({ 376, 731 });
		ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

		char nameBuf[128] = {};
		for (int32 i = 0; i < entity->name.GetLength() && i < 128; i++)
		{
			nameBuf[i] = entity->name[i];
		}

		if (ImGui::InputText("Name", nameBuf, sizeof(nameBuf)))
		{
			entity->name = CString(nameBuf);
		}

		ImGui::SameLine();
		ImGui::Checkbox("Active", &entity->active);

		if (ImGui::CollapsingHeader("Local Transform"))
		{
			DisplayTransform(&entity->transform);
		}

		if (entity->lightComp.active && ImGui::CollapsingHeader("Light Component"))
		{
			LightComponent* light = &entity->lightComp;
			//light->active;
			light->type = DisplayFancyEnum<LightType>("Type", light->type);
			ImGui::ColorEdit3("Colour", light->colour.ptr);
			ImGui::DragFloat("Intensity", &light->intensity, 0.1f, 0.0f, 100.0f, "%.3f");
		}

		ImGui::End();
		ImGui::PopStyleVar();
	}

	static void DisplayPerformanceWindow(EditorState* es, GameState* gs)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
		ImGui::Begin("Performance", &es->showPerformanceWindow);

		ImGui::Checkbox("Vsync", &es->vsync);

		CString stringBuffer;
		if (ImGui::CollapsingHeader("Permanent Memory"))
		{
			ImGui::Text("Current permanent memory allocated MB:");
			ImGui::SameLine();
			uint64 permanentTotalMB = GameMemory::GetTheTotalAmountOfPermanentMemoryAllocated() / 1000000;
			stringBuffer.Clear();
			stringBuffer.Add(permanentTotalMB);
			ImGui::Text(stringBuffer.GetCStr());

			ImGui::Text("Current permanent memory used MB:");
			ImGui::SameLine();
			uint64 permanentBytes = GameMemory::GetTheAmountOfPermanentMemoryUsed();
			uint32 permanentUsedMB = (uint32)(permanentBytes / 1000000);
			stringBuffer.Clear();
			stringBuffer.Add(permanentUsedMB);
			ImGui::Text(stringBuffer.GetCStr());

			ImGui::Text("Current permanent memory available MB:");
			ImGui::SameLine();
			uint64 permanentAvaialbeMB =
				(GameMemory::GetTheTotalAmountOfPermanentMemoryAllocated() -
					GameMemory::GetTheAmountOfPermanentMemoryUsed()) / 1000000;
			stringBuffer.Clear();
			stringBuffer.Add(permanentAvaialbeMB);
			ImGui::Text(stringBuffer.GetCStr());

			ImGui::Text("Current permanent memory used %%:");
			ImGui::SameLine();
			real64 permanentPercent = (real64)permanentBytes / (real64)GameMemory::GetTheTotalAmountOfPermanentMemoryAllocated();
			stringBuffer.Clear();
			stringBuffer.Add((real32)permanentPercent);
			ImGui::Text(stringBuffer.GetCStr());
		}
		if (ImGui::CollapsingHeader("Transient Memory"))
		{
			ImGui::Text("Current transient memory allocated MB:");
			ImGui::SameLine();
			uint64 transientTotalMB = GameMemory::GetTheTotalAmountOfTransientMemoryAllocated() / 1000000;
			stringBuffer.Clear();
			stringBuffer.Add(transientTotalMB);
			ImGui::Text(stringBuffer.GetCStr());

			ImGui::Text("Current transient memory used MB:");
			ImGui::SameLine();
			uint64 transientBytes = GameMemory::GetTheAmountOfTransientMemoryUsed();
			uint32 transientMB = (uint32)(transientBytes / 1000000);
			stringBuffer.Clear();
			stringBuffer.Add(transientMB);
			ImGui::Text(stringBuffer.GetCStr());

			ImGui::Text("Current transient memory available MB:");
			ImGui::SameLine();
			uint64 transientAvaialbeMB =
				(GameMemory::GetTheTotalAmountOfTransientMemoryAllocated() -
					GameMemory::GetTheAmountOfTransientMemoryUsed()) / 1000000;
			stringBuffer.Clear();
			stringBuffer.Add(transientAvaialbeMB);
			ImGui::Text(stringBuffer.GetCStr());

			ImGui::Text("Current transient memory used %%:");
			ImGui::SameLine();
			real64 transientPercent = (real64)transientBytes / (real64)GameMemory::GetTheTotalAmountOfTransientMemoryAllocated();
			stringBuffer.Clear();
			stringBuffer.Add((real32)transientPercent);
			ImGui::Text(stringBuffer.GetCStr());
		}
		if (ImGui::CollapsingHeader("Frame Timings"))
		{
			ImGui::Text("Frame time:");
			ImGui::SameLine();
			ImGui::Text(CString("").Add(gs->dt * 1000).GetCStr());

			for (int32 i = 0; i < ArrayCount(es->frameTimes) - 1; i++)
			{
				es->frameTimes[i] = es->frameTimes[i + 1];
			}
			es->frameTimes[ArrayCount(es->frameTimes) - 1] = gs->dt * 1000;

			ImGui::PlotLines("Frame time", es->frameTimes, ArrayCount(es->frameTimes), 0, 0, 0, 30, ImVec2(ImGui::GetWindowSize().x, 64), 4);
		}

		ImGui::End();
		ImGui::PopStyleVar();
	}

	static void DisplayContentWindow(EditorState* es, GameState* gs)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
		ImGui::SetNextWindowPos({ 0,18 + 731 });
		ImGui::SetNextWindowSize({ 1900, 230 });
		ImGui::Begin("Content", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);



		ImGui::End();
		ImGui::PopStyleVar();
	}

	static void DisplayToolBar(EditorState* es)
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
				if (ImGui::MenuItem("Performance")) { es->showPerformanceWindow = true; }
				if (ImGui::MenuItem("Render Settings")) { es->showRenderSettingsWindow = true; }
				//if (ImGui::MenuItem("Main window")) { es->windowOpen = true; }
				//if (ImGui::MenuItem("Physics")) { es->physicsWindowOpen = true; }
				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}
	}

	template<uint32 size>
	static void SaveLargeStringToFile(const CString& fileName, LargeString<size>* fileData)
	{
		PlatformFile file = {};
		file.data = (void*)fileData->GetCStr();
		file.size_bytes = fileData->GetLength() * sizeof(char);

		DEBUGWriteFile(file, fileName);
	}

	static void SaveGameStateTextFile(GameState* gs)
	{
		gs->BeginEntityLoop();

		LargeString<10000>* fileData = GameMemory::PushTransientStruct<LargeString<10000>>();

		while (Entity* entity = gs->GetNextEntity())
		{
			fileData->Add("Entity:\n");
			fileData->Add("\tname=").Add(entity->name).Add("\n");
			fileData->Add("\tactive=").Add(entity->active).Add("\n");
			fileData->Add("\tTransform:\n");
			fileData->Add("\t\tposition=").Add(ToString(entity->transform.position)).Add("\n");
			fileData->Add("\t\torientation=").Add(ToString(entity->transform.orientation)).Add("\n");
			fileData->Add("\t\tscale=").Add(ToString(entity->transform.scale)).Add("\n");

			if (entity->renderComp.active)
			{
				fileData->Add("\tRenderComponent:\n");
				fileData->Add("\t\tflags=").Add(entity->renderComp.flags).Add("\n");
				fileData->Add("\t\tmodel=").Add(entity->renderComp.modelId.ToString()).Add("\n");
			}

			if (entity->lightComp.active)
			{
				fileData->Add("\tLightComponent:\n");
				fileData->Add("\t\tcolour=").Add(ToString(entity->lightComp.colour)).Add("\n");
				fileData->Add("\t\tintensity=").Add(entity->lightComp.intensity).Add("\n");
			}
		}

		SaveLargeStringToFile("../Assets/Raw/Worlds/demo.txt", fileData);
	}

	void UpdateEditor(GameState* gs, RenderState* rs, AssetState* as, EditorState* es, Input* input, PlatformState* ws, EntityRenderGroup* renderGroup)
	{
		BeginEditorNewFrame();

		if (IsKeyJustDown(input, f5))
		{
			es->inGame = !es->inGame;
		}

		if (!es->inGame)
		{
			ConstructRenderGroup(gs, renderGroup);

			input->mouse_locked = false;
			if (input->mb2)
			{
				input->mouse_locked = true;
				OperateCamera(es, input, gs->dt);
			}
			else
			{
				if (es->selectedEntity)
				{
					es->gizmo.Operate(es->camera, es->selectedEntity, input);
				}

				DisplayToolBar(es);
				DisplayWorldWindow(es, gs);

				if (es->selectedEntity)
				{
					DisplayEntityInspector(es->selectedEntity);
				}

				if (es->showPerformanceWindow)
				{
					DisplayPerformanceWindow(es, gs);
				}

				//es->nodeWindow.Show(input);
				//DisplayContentWindow(es, gs);

#if 0
				//es->nodeWindow.Show(input);
				ImGui::Begin("Editor");

				if (ImGui::Button("Save"))
				{
					SaveGameStateTextFile(gs);
				}

				if (ImGui::CollapsingHeader("Entities"))
				{
					gs->BeginEntityLoop();
					while (Entity* entity = gs->GetNextEntity())
					{
						if (ImGui::TreeNode(entity->name.GetCStr()))
						{
							if (ImGui::Button("Select"))
							{
								es->selectedEntity = entity;
							}

							DisplayTransform(&entity->transform);
							ImGui::TreePop();
						}
					}
				}

				if (ImGui::CollapsingHeader("Player"))
				{
					if (ImGui::Button("Rotate Y"))
					{
						gs->camera.transform.GlobalRotateY(0.314159f / 2.0f);
					}
				}

				if (ImGui::CollapsingHeader("Performance"))
				{
					ImGui::Checkbox("Vsync", &es->vsync);

					ImGui::Text("Start up time: ");
					ImGui::SameLine();
					ImGui::Text(CString("").Add(gs->dt * 1000).GetCStr());
					//ImGui::Text(("Last frame time: " + std::to_string(input->dt * 1000)).c_str());

					for (int32 i = 0; i < ArrayCount(es->frameTimes) - 1; i++)
					{
						es->frameTimes[i] = es->frameTimes[i + 1];
					}
					es->frameTimes[ArrayCount(es->frameTimes) - 1] = gs->dt * 1000;

					ImGui::PlotLines("Frame time", es->frameTimes, ArrayCount(es->frameTimes), 0, 0, 0, 30, ImVec2(128, 64), 4);
				}

				float size = 512;
				//ImGui::Image(rs->shadowCascades.shaderView, ImVec2(size, size));
				ImGui::Image(rs->reductionTargets[2].shaderView, ImVec2(size, size));

				ImGui::End();
#endif
			}



			renderGroup->mainCamera = es->camera;


			//ImGui::ShowDemoWindow();
		}
		else
		{
			UpdateGame(gs, as, ws, input);
			ConstructRenderGroup(gs, renderGroup);
		}
	}
}
