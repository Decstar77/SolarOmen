#include "SolarEditor.h"

namespace cm
{
	void InitializeEditorState(GameState* gs, RenderState* rs, AssetState* as, EditorState* es, PlatformState* ws)
	{
		ImGui::CreateContext();
		ImNodes::CreateContext();
		ImGui::StyleColorsDark();

		ImVec4* colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_WindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.94f);
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


	void OperateGizmo(EditorState* es, GameState* gs, PlatformState* ws, Input* input)
	{
		ImGuizmo::Enable(true);
		ImGuiIO& io = ImGui::GetIO();
		ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

		if (IsKeyJustDown(input, e) && !input->mb1)
		{
			es->op = ImGuizmo::OPERATION::SCALE;
		}
		if (IsKeyJustDown(input, r) && !input->mb1)
		{
			es->op = ImGuizmo::OPERATION::ROTATE;
		}
		if (IsKeyJustDown(input, t) && !input->mb1)
		{
			es->op = ImGuizmo::OPERATION::TRANSLATE;
		}

		if (IsKeyJustDown(input, tlda) && !input->mb1)
		{
			es->md = (es->md == ImGuizmo::MODE::LOCAL) ? ImGuizmo::MODE::WORLD : ImGuizmo::MODE::LOCAL;
		}

		bool snapping = true;

		Vec3f snap_amount = Vec3f(VOXEL_IMPORT_SCALE);
		if (es->op == ImGuizmo::OPERATION::ROTATE)
		{
			snap_amount = Vec3f(15);
		}
		if (input->ctrl)
		{
			snapping = false;

		}

		Mat4f view = Inverse(es->camera.transform.CalculateTransformMatrix());
		Mat4f proj = PerspectiveLH(DegToRad(gs->camera.yfov), ws->aspect, gs->camera.near_, gs->camera.far_);

		Mat4f world_mat = es->selectedEntity->transform.CalculateTransformMatrix();
		Mat4f delta;
#if 0
		if (es->selectedCollider)
		{
			world_mat = es->selectedCollider->mat;
			world_mat[3][3] = 1;
			world_mat = ScaleCardinal(world_mat, es->selectedCollider->extents);

			AABB box = CreateAABBContainingOBB(*es->selectedCollider);
			float bounds[] = { box.min.x, box.min.y, box.min.z, box.max.x, box.max.y ,box.max.z };

			ImGuizmo::Manipulate(view.ptr, proj.ptr, es->op, es->md, world_mat.ptr, nullptr, 0, 0);

			Vec3f pos;
			Vec3f euler;
			Vec3f scale;
			ImGuizmo::DecomposeMatrixToComponents(world_mat.ptr, pos.ptr, euler.ptr, scale.ptr);

			es->selectedCollider->center = pos;
			es->selectedCollider->basis = RemoveScaleFromRotationMatrix(Mat3f(world_mat));
			es->selectedCollider->extents = scale;
		}
#endif
		{
			ImGuizmo::Manipulate(view.ptr, proj.ptr, es->op, es->md, world_mat.ptr, delta.ptr, snapping ? (snap_amount.ptr) : nullptr);

			Vec3f pos;
			Vec3f euler;
			Vec3f scale;
			ImGuizmo::DecomposeMatrixToComponents(world_mat.ptr, pos.ptr, euler.ptr, scale.ptr);
			Quatf qori = EulerToQuat(euler);

			// @NOTE: These equals are done to prevent floating point errors that trick 
			//		: the undo system for the game state to think that a change has occurred

			if (!Equal(pos, es->selectedEntity->transform.position))
			{
				es->selectedEntity->transform.position = pos;
			}

			if (!Equal(qori, es->selectedEntity->transform.orientation))
			{
				es->selectedEntity->transform.orientation = qori;
			}

			if (!Equal(scale, es->selectedEntity->transform.scale))
			{
				es->selectedEntity->transform.scale = scale;
			}
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
					OperateGizmo(es, gs, ws, input);
				}


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
