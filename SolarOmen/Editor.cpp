#include "Editor.h"
#if 0
#include "Debug.h"
#include <stack>
#include <queue>

namespace cm
{
	static void InitUndoSystems(GameState* gs, EditorState* es)
	{
		auto baseAction = EditorAction::UNKNOWN;
		es->actions.Init(&baseAction);
		es->selectionUndo.Init(&es->selected_entity);
		es->gameStateUndo.Init(gs);
	}

	static inline void DeselectAll(EditorState* es)
	{
		es->selectedCollider = nullptr;
		es->selected_entity = nullptr;
		es->selectedPrefab = nullptr;
	}

	static void LoadAGameWorld(GameState* gs, RenderState* rs, AssetState* as, EditorState* es, const CString& path)
	{
		if (LoadWorldFromTextFile(gs, as, rs, path.GetCStr()))
		{
			es->currentGameFile = path;
			DeselectAll(es);
			InitUndoSystems(gs, es);
			LOG("Loaded: " << path.GetCStr());
		}
		else
		{
			LOG("Could not load: " << path.GetCStr());
		}
	}

	void InitializeEditorState(GameState* gs, RenderState* rs, AssetState* as, EditorState* es, PlatformState* ws)
	{
		// Init imgui
		ImGui::CreateContext();
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
		es->camera.far_ = 100.0f;
		es->camera.near_ = 0.3f;
		es->camera.yfov = 45.0f;
		es->camera.aspect = (real32)ws->client_width / (real32)ws->client_height;

		es->currentGameFile = {};

		es->vsync = true;

		InitUndoSystems(gs, es);
	}

	void RenderEditor(EditorState* es)
	{
		if (es->mode != EditorMode::GAME)
		{
			DrawEditorFrame(es);
		}
	}

	void BeginEditorNewFrame(EditorState* es)
	{
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();
	}

	void DrawEditorFrame(EditorState* es)
	{
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}

	void OperateCamera(EditorState* es, Input* input)
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

		real32 dtime = input->dt;
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

		Mat4f world_mat = es->selected_entity->transform.CalculateTransformMatrix();
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

			if (!Equal(pos, es->selected_entity->transform.position))
			{
				es->selected_entity->transform.position = pos;
			}

			if (!Equal(qori, es->selected_entity->transform.orientation))
			{
				es->selected_entity->transform.orientation = qori;
			}

			if (!Equal(scale, es->selected_entity->transform.scale))
			{
				es->selected_entity->transform.scale = scale;
			}
		}
	}

	Entity* SelectEntity(EditorState* es, GameState* gs, RenderState* rs, AssetState* as, PlatformState* ws, Input* input)
	{
		Camera temp = gs->camera;
		temp.transform = es->camera.transform;
		Ray ray = temp.ShootRayFromScreen(ws, input->mouse_pixl);

		Entity* closest_entity = nullptr;
		real32 closest_dist = 10000.0f;
		// @TODO: Entity loop
		for (int32 i = 0; i < ArrayCount(gs->entites); i++)
		{
			Entity* entity = &gs->entites[i];
			if (IsValidEntity(gs, entity) && entity->active)
			{
				AABB bounding_box = GetEntityBoundingBox(entity);

				real32 dist;
				if (RaycastAABB(ray, bounding_box, &dist))
				{
					if (dist < closest_dist)
					{
						if (IsValidMesh(rs, entity->render.mesh))
						{
							MeshData* mesh_data = LookUpMeshData(as, entity->render.mesh);
							Mat4f entity_tranform = entity->transform.CalculateTransformMatrix();

							for (int32 index = 0; index < mesh_data->indices_count; index += 3)
							{
								int32 vertex_index = mesh_data->indices[index] * mesh_data->packed_stride;

								Vec3f v1 = Vec3f(
									mesh_data->packed_vertices[vertex_index],
									mesh_data->packed_vertices[vertex_index + 1],
									mesh_data->packed_vertices[vertex_index + 2]);

								vertex_index = mesh_data->indices[index + 1] * mesh_data->packed_stride;

								Vec3f v2 = Vec3f(
									mesh_data->packed_vertices[vertex_index],
									mesh_data->packed_vertices[vertex_index + 1],
									mesh_data->packed_vertices[vertex_index + 2]);


								vertex_index = mesh_data->indices[index + 2] * mesh_data->packed_stride;

								Vec3f v3 = Vec3f(
									mesh_data->packed_vertices[vertex_index],
									mesh_data->packed_vertices[vertex_index + 1],
									mesh_data->packed_vertices[vertex_index + 2]);

								v1 = Vec3f(Vec4f(v1, 1.0f) * entity_tranform);
								v2 = Vec3f(Vec4f(v2, 1.0f) * entity_tranform);
								v3 = Vec3f(Vec4f(v3, 1.0f) * entity_tranform);

								Triangle tri = CreateTriangle(v1, v2, v3);

								real32 tridist;
								if (RaycastTriangle(ray, tri, &tridist))
								{
									if (tridist < closest_dist)
									{
										closest_dist = tridist;
										closest_entity = entity;
									}
								}
							}
						}
						else
						{
							closest_dist = dist;
							closest_entity = entity;
						}
					}
				}
			}
		}

		return closest_entity;
	}

	static inline void ComboOfMeshes(RenderState* rs, AssetState* as, TransientState* ts, int32* meshIndex)
	{
		ImGui::Separator();

		LargeString meshStream = ts->stringMemory.GetLargeString();
		for (int32 i = 0; i < ArrayCount(as->meshesData); i++)
		{
			if (rs->meshes[i].vertex_buffer)
			{
				MeshData* meshData = &as->meshesData[i];
				meshStream.Add(meshData->name).Add('\0');
			}
		}

		ImGui::Combo("Meshes", meshIndex, meshStream.GetCStr());
		ts->stringMemory.FreeLargeString();
	}

	static inline void ComboOfEntityType(RenderState* rs, TransientState* ts, int32* typeIndex)
	{
		LargeString typeStream = ts->stringMemory.GetLargeString();

		for (int32 i = 1; i < (int32)EntityType::ENTITY_TYPE_COUNT; i++)
		{
			typeStream.Add(EntityTypeToString((EntityType)i).c_str()).Add('\0');
		}

		ImGui::Combo("Entity type", typeIndex, typeStream.GetCStr());
		ts->stringMemory.FreeLargeString();
	}


	static inline void ComboOfTextures(RenderState* rs, AssetState* as, TransientState* ts, int32* textureIndex)
	{
		LargeString textureStream = ts->stringMemory.GetLargeString();

		for (int32 i = 0; i < ArrayCount(as->texturesData); i++)
		{
			if (IsValidTexture(rs, i))
			{
				TextureData* textureData = &as->texturesData[i];
				textureStream.Add(textureData->name).Add('\0');
			}
		}

		ImGui::Combo("Textures", textureIndex, textureStream.GetCStr());
		ts->stringMemory.FreeLargeString();
	}

	static inline void ComboOfShaders(RenderState* rs, AssetState* as, TransientState* ts, int32* shaderIndex)
	{
		LargeString shaderStream = ts->stringMemory.GetLargeString();
		for (int32 i = 0; i < ArrayCount(as->shadersData); i++)
		{
			if (IsValidShader(rs, i))
			{
				ShaderData* shaderData = &as->shadersData[i];
				shaderStream.Add(shaderData->name).Add('\0');
			}
		}

		ImGui::Combo("Shaders", shaderIndex, shaderStream.GetCStr());
		ts->stringMemory.FreeLargeString();
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

	static void DisplayEntity(GameState* gs, RenderState* rs, AssetState* as, TransientState* ts, Entity* selectedEntity)
	{
		EntityType type = selectedEntity->type;

		if (ImGui::TreeNode("General"))
		{
			ImGui::Text(EntityTypeToString(type).c_str());
			ImGui::Checkbox("Active", &selectedEntity->active);
			rs->buildBVH = ImGui::Button("Build BVH");

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Transform"))
		{
			DisplayTransform(&selectedEntity->transform);

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Colliders"))
		{
			if (ImGui::Button("Generate"))
			{
				OBB collider = CreateOBB(selectedEntity->object_space_bounding_box);
				selectedEntity->collision.box = collider;
				selectedEntity->collision.type = ColliderType::BOX;
			}

			if (selectedEntity->collision.type == ColliderType::BOX)
			{
				OBB* collider = &selectedEntity->collision.box;
				Transform colliderTransform;
				colliderTransform.position = collider->center;
				colliderTransform.orientation = Mat3ToQuat(collider->basis.mat);
				colliderTransform.scale = collider->extents;

				DisplayTransform(&colliderTransform, 0.01f);

				collider->center = colliderTransform.position;
				collider->basis.mat = QuatToMat3(colliderTransform.orientation);
				collider->extents = colliderTransform.scale;

				DEBUGDrawOBB(GetEntityBoxCollider(selectedEntity));
			}

			ImGui::TreePop();
		}

		if (IsEntityPhsyicsEnabled(selectedEntity) && ImGui::TreeNode("Rigid Body"))
		{
			RigidBodyComponent* ri = &selectedEntity->rigidBody;
			real32 mass = ri->invMass == 0.0f ? 0.0f : 1.0f / ri->invMass;
			ImGui::DragFloat("Mass", &mass, 0.1f, 0.0);
			ri->invMass = mass == 0.0f ? 0.0f : 1.0f / mass;

			ImGui::DragFloat("Elasticity", &ri->elasticity, 0.01f, 0.0, 1.0f);
			ImGui::DragFloat("Friction", &ri->friction, 0.01f, 0.0, 1.0f);

			ImGui::DragFloat3("Linear velocity", ri->linearVelocity.ptr, 0.1f);
			ImGui::DragFloat3("Angular velocity", ri->angularVelocity.ptr, 0.1f);
			ImGui::DragFloat3("Forces", ri->forces.ptr, 0.1f);
			ImGui::DragFloat3("Torque", ri->torque.ptr, 0.1f);

			ImGui::TreePop();
		}

		if (type == EntityType::POINT_LIGHT || type == EntityType::DIR_LIGHT || type == EntityType::SPOT_LIGHT)
		{
			if (ImGui::TreeNode("Light settings"))
			{
				ImGui::ColorEdit3("Colour", selectedEntity->light.colour.ptr);
				ImGui::DragFloat("Intensity", &selectedEntity->light.intensity, 0.01f);

				ImGui::TreePop();
			}
		}
		if (type == EntityType::PARTICLE_EMITTER)
		{
			if (ImGui::TreeNode("Particle Emitter settings"))
			{
				ParticleEmitterPart* emitter = &selectedEntity->particlePart;

				ImGui::ColorEdit3("Starting colour", emitter->startingColour.ptr);
				ImGui::ColorEdit3("Ending colour", emitter->endingColour.ptr);
				ImGui::TreePop();

			}
		}
		else if (type == EntityType::ENVIRONMENT)
		{
			if (ImGui::TreeNode("Render settings"))
			{
				RenderComponent* render = &selectedEntity->render;

				MeshData* entityMeshData = LookUpMeshData(as, render->mesh);
				ImGui::Text(("Total voxel boxes: " + std::to_string(entityMeshData->shadowBoxCount)).c_str());

				static bool showBoxes = false;
				ImGui::Checkbox("Show voxel boxes", &showBoxes);
				if (showBoxes)
				{
					for (int32 voxelBox = 0; voxelBox < entityMeshData->shadowBoxCount; voxelBox++)
					{
						OBB box = TransformOBB(CreateOBB(entityMeshData->shadowBoxes[voxelBox]),
							selectedEntity->transform.CalculateTransformMatrix());

						//box.basis.mat = QuatToMat3(es->selected_entity->transform.orientation) * box.basis.mat;

						DEBUGDrawOBB(box);
					}
				}

				int32 meshIndex = render->mesh - 1;
				ComboOfMeshes(rs, as, ts, &meshIndex);
				SetEntityMesh(as, selectedEntity, meshIndex + 1);

				int32 textureIndex = render->texture - 1;
				ComboOfTextures(rs, as, ts, &textureIndex);
				render->texture = textureIndex + 1;

				int32 shaderIndex = render->shader - 1;
				ComboOfShaders(rs, as, ts, &shaderIndex);
				render->shader = shaderIndex + 1;

				bool castShadow = (bool)(render->flags & (uint32)RenderFlags::CAST_SHADOW);
				bool recShadow = (bool)(render->flags & (uint32)RenderFlags::RECEIVES_SHADOW);

				ImGui::Checkbox("Cast shadow", &castShadow);
				ImGui::Checkbox("Recives shadow", &recShadow);

				render->flags = castShadow ? (uint32)(RenderFlags::CAST_SHADOW) : 0;
				render->flags |= recShadow ? (uint32)(RenderFlags::RECEIVES_SHADOW) : 0;

				ImGui::TreePop();
			}
		}
	}

	void ProcessUndoSystems(GameState* gs, EditorState* es, Input* input)
	{
		if (IsAnyKeyJustUp(input))
		{
			EditorAction action = EditorAction::UNKNOWN;

			if (es->selected_entity != *es->selectionUndo.GetCurrent())
			{
				LOG("Selection");
				action = EditorAction::SELECTION_CHANGE;
				es->selectionUndo.Do(&es->selected_entity);
			}

			if (!CompareGameStates(gs, es->gameStateUndo.GetCurrent()))
			{
				LOG("GameState");
				action = EditorAction::GAMESTATE_CHANGE;
				es->gameStateUndo.Do(gs);
			}

			if (action != EditorAction::UNKNOWN)
			{
				es->actions.Do(&action);
			}
		}

		if (input->ctrl && input->shift && IsKeyJustDown(input, z))
		{
			EditorAction action;
			es->actions.Redo(&action);

			switch (action)
			{
			case EditorAction::SELECTION_CHANGE:
			{
				es->selectionUndo.Redo(&es->selected_entity);
			}break;
			case EditorAction::GAMESTATE_CHANGE:
			{
				es->gameStateUndo.Redo(gs);
			}break;
			}
		}
		else if (input->ctrl && IsKeyJustDown(input, z))
		{
			EditorAction action = *es->actions.GetCurrent();
			es->actions.Undo();
			switch (action)
			{
			case EditorAction::SELECTION_CHANGE:
			{
				es->selected_entity = *es->selectionUndo.Undo();
			}break;
			case EditorAction::GAMESTATE_CHANGE:
			{
				*gs = *es->gameStateUndo.Undo();
			}break;
			}
		}
	}

	void UpdateEditor(GameState* gs, RenderState* rs, AssetState* as, EditorState* es, Input* input, PlatformState* ws, TransientState* ts)
	{
		if (es->mode == EditorMode::FREE && input->ctrl && IsKeyJustDown(input, s))
		{
			//CString path = "../Assets/Processed/Scenes/"; // @TODO: Globbal !!
			CString path = es->currentGameFile;
			if (path.GetLength() != 0)
			{
				SaveWorldToTextFile(gs, rs, as, ts, path.GetCStr());
				LOG("GAME SAVED");
			}
			else
			{
				LOG("No current scene!!");
			}

		}

		if ((es->mode == EditorMode::GAME || es->mode == EditorMode::FREE))
		{
			if (IsKeyJustDown(input, f4))
			{
				LoadAGameWorld(gs, rs, as, es, "../Assets/Processed/Worlds/Palette.txt");
			}

			if (IsKeyJustDown(input, f3))
			{
				LoadAGameWorld(gs, rs, as, es, "../Assets/Processed/Worlds/PhysicsTest2_PenetrationConstraint.txt");
			}

			if (IsKeyJustDown(input, f2))
			{
				if (es->mode == EditorMode::FREE)
				{
					es->mode = EditorMode::GAME;
				}
				else
				{
					es->mode = EditorMode::FREE;
				}
			}
		}

		if (es->mode == EditorMode::GAME)
		{
			UpdateGame(gs, as, ws, input, ts);
			return;
		}

		if (input->f7)
		{
			DEBUGStepPhysics(gs, ts, input);
		}
		if (IsKeyJustDown(input, f8))
		{
			DEBUGStepPhysics(gs, ts, input);
		}

		BeginEditorNewFrame(es);

		Camera* camera = &es->camera;
		if (input->mb2)
		{
			OperateCamera(es, input);
		}
		else
		{
			input->mouse_locked = false;

			if (ImGui::BeginMainMenuBar())
			{
				if (ImGui::BeginMenu("File"))
				{
					if (ImGui::MenuItem("New")) {}
					if (ImGui::MenuItem("Open", "Ctrl+O")) {
						CString path = PlatformOpenNFileDialogAndReturnPath();
						LoadAGameWorld(gs, rs, as, es, path);
					}
					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("View"))
				{
					if (ImGui::MenuItem("Main window")) { es->windowOpen = true; }
					if (ImGui::MenuItem("Physics")) { es->physicsWindowOpen = true; }
					ImGui::EndMenu();
				}

				ImGui::EndMainMenuBar();
			}

			//ImGui::ShowDemoWindow();
			//ImGui::SetNextWindowPos(ImVec2(0, 0));
			//ImGui::SetNextWindowSize(ImVec2(375, 1000));

			if (input->shift && IsKeyJustDown(input, a))
			{
				ImGui::OpenPopup("Add Entity");
			}

			ImVec2 center(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f);
			ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			if (ImGui::BeginPopupModal("Add Entity", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Separator();

				static int32 typeIndex = 0;
				ComboOfEntityType(rs, ts, &typeIndex);

				static int32 meshIndex = 0;
				static int32 textureIndex = 0;
				static bool32 simulatePhsyics = 0;

				static real32 mass = 0;

				EntityType type = (EntityType)(typeIndex + 1);

				if (type == EntityType::ENVIRONMENT)
				{
					ComboOfMeshes(rs, as, ts, &meshIndex);
					ComboOfTextures(rs, as, ts, &textureIndex);
				}

				ImGui::Checkbox("Simulate Physics", &simulatePhsyics);
				if (simulatePhsyics)
				{
					ImGui::InputFloat("Mass", &mass);
				}

				ImGui::Separator();
				if (ImGui::Button("Create", ImVec2(160, 0))) {

					Entity* entity = CreateEntity(gs);
					entity->type = (EntityType)(typeIndex + 1);
					entity->render.flags = (uint32)RenderFlags::RECEIVES_SHADOW | (uint32)RenderFlags::CAST_SHADOW;

					if (entity->type == EntityType::ENVIRONMENT)
					{
						SetEntityMesh(as, entity, meshIndex + 1);
						SetEntityShader(as, entity, rs->pbr_shader);
						SetEntityTexture(as, entity, textureIndex + 1);
					}

					if (simulatePhsyics)
					{
						entity->flags = (EntityFlag)((int32)entity->flags | (int32)EntityFlag::SIMULATE_PHYSICS);
						entity->rigidBody.invMass = mass == 0.0f ? 0.0f : 1.0f / mass;
					}

					ImGui::CloseCurrentPopup();
				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel", ImVec2(160, 0))) { ImGui::CloseCurrentPopup(); }


				ImGui::EndPopup();
			}

			if (es->physicsWindowOpen && ImGui::Begin("Physics", &es->physicsWindowOpen, 0))
			{
				if (ImGui::Button("Step sim a shit ton"))
				{
					for (int32 i = 0; i < 100000; i++)
					{
						ZeroStruct(ts);
						DEBUGStepPhysics(gs, ts, input);
					}
				}
				ImGui::End();
			}

			if (es->windowOpen && ImGui::Begin("Editor", &es->windowOpen, 0))
			{
				if (es->selected_entity && ImGui::CollapsingHeader("Selected entity"))
				{
					DisplayEntity(gs, rs, as, ts, es->selected_entity);
				}

				if (ImGui::CollapsingHeader("World"))
				{
					if (ImGui::TreeNode("Entities"))
					{
						for (int32 entityIndex = 0; entityIndex < ArrayCount(gs->entites); entityIndex++)
						{
							Entity* entity = &gs->entites[entityIndex];

							if (IsValidEntity(gs, entity))
							{
								if (entity->name.GetLength() == 0)
								{
									entity->name = ("Entity " + std::to_string(entityIndex)).c_str();
								}

								CString def = CString(entity->name).Add(": ").Add(EntityTypeToString(entity->type).c_str());

								if (entity == es->selected_entity)
								{
									ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor::HSV(140.0f / 360.0f, 0.94f, 0.7f));
									ImGui::Text(def.GetCStr());
									ImGui::PopStyleColor(1);
								}
								else
								{
									ImGui::Text(def.GetCStr());
								}

								ImGui::SameLine();
								ImGui::PushID(entityIndex);
								if (ImGui::SmallButton("Select"))
								{
									es->selected_entity = entity;
									LOG("s");
								}
								ImGui::PopID();
							}
						}

						ImGui::TreePop();
					}

					if (ImGui::TreeNode("Sectors"))
					{
						if (ImGui::Button("Added world sector"))
						{
							CreateWorldSector(gs);
						}

						for (int32 worldSectorIndex = 0; worldSectorIndex < gs->worldSectorCount; worldSectorIndex++)
						{
							WorldSector* worldSector = &gs->worldSectors[worldSectorIndex];
							CString name = CString("WorldSector: ").Add(worldSectorIndex);

							if (ImGui::TreeNode(name.GetCStr()))
							{
								ImGui::InputInt("Neighbour 1 ", &worldSector->neighbour1);
								ImGui::InputInt("Neighbour 2", &worldSector->neighbour2);

								worldSector->neighbour1 = Clamp(worldSector->neighbour1, 0, gs->worldSectorCount - 1);
								worldSector->neighbour2 = Clamp(worldSector->neighbour2, 0, gs->worldSectorCount - 1);

								Vec3f center = GetAABBCenter(worldSector->boundingBox);
								Vec3f extents = GetAABBRadius(worldSector->boundingBox);

								ImGui::DragFloat3("Center", center.ptr, 0.25);
								ImGui::DragFloat3("Extents", extents.ptr, 0.25);

								worldSector->boundingBox = CreateAABBFromCenterRadius(center, extents);

								ImGui::TreePop();
							}
						}
						ImGui::TreePop();
					}
				}

				if (ImGui::CollapsingHeader("Renderer settings"))
				{
					ImGui::DragFloat("Bloom threshold", &rs->bloomThreshold, 0.01f, 0.0f, 100.0f);
					ImGui::DragFloat("Bloom Softness", &rs->bloomSoftness, 0.01f, 0.0f, 100.0f);
				}

				if (ImGui::CollapsingHeader("Performance"))
				{

					ImGui::Checkbox("Vsync", &es->vsync);
					ImGui::Text(("Start up time: " + std::to_string(es->startUpTime)).c_str());
					ImGui::Text(("Last frame time: " + std::to_string(input->dt * 1000)).c_str());

					for (int32 i = 0; i < ArrayCount(es->frameTimes) - 1; i++)
					{
						es->frameTimes[i] = es->frameTimes[i + 1];
					}
					es->frameTimes[ArrayCount(es->frameTimes) - 1] = input->dt * 1000;

					ImGui::PlotLines("Frame time", es->frameTimes, ArrayCount(es->frameTimes), 0, 0, 0, 30, ImVec2(128, 64), 4);
				}
#if 0
				if (ImGui::CollapsingHeader("Shadow bvh"))
				{
					ImGui::Text(("Node count: " + std::to_string(rs->bvh.nodeCount)).c_str());
					static bool32 leafs = false;
					ImGui::Checkbox("Display leafs", &leafs);
					static int32 num = -1;
					ImGui::InputInt("Node number", &num);
					bool32 isLeaf = num > 0 ? rs->bvh.nodes[num].IsLeaf() : false;
					ImGui::Text(("Is leaf: " + std::to_string(isLeaf)).c_str());
					DEBUGDrawBVH(rs, &rs->bvh, leafs, num);

					Ray ray = CreateRay(gs->testEntity1->transform.position, Normalize(gs->testEntity2->transform.position - gs->testEntity1->transform.position));
					DEBUGDrawLine(rs, gs->testEntity1->transform.position, gs->testEntity2->transform.position);

					ImGui::InputInt("Max prim count: ", &rs->bvh.maxPrimCount);
					{
						int32 hitCount = 0;
						std::stack<int32> stack;
						stack.push(0);
						while (!stack.empty())
						{
							BVHNode* node = &rs->bvh.nodes[stack.top()];
							stack.pop();
							real32 dist;
							//if (!RaycastAABB(ray, node->box, &dist))
							//	continue;

							if (node->IsLeaf())
							{
								hitCount += node->primCount;
							}
							else
							{
								stack.push(node->firstIndex);
								stack.push(node->firstIndex + 1);
								hitCount++;
							}
						}

						ImGui::Text(("Trace count: " + std::to_string(hitCount)).c_str());
					}

					ImGui::Checkbox("Blur shadows", &rs->blurShadows);
				}
#endif



				ImGui::Checkbox("Show bounding boxes", &es->show_bounding_boxes);
				if (es->show_bounding_boxes)
				{
					// @TODO: Entity loop
					for (int32 i = 0; i < ArrayCount(gs->entites); i++)
					{
						Entity* entity = &gs->entites[i];
						if (IsValidEntity(gs, entity) && entity->active)
						{
							AABB world_space_bounding_box = GetEntityBoundingBox(entity);
							DEBUGDrawAABB(world_space_bounding_box);
						}
					}
				}

				ImGui::Checkbox("Show all colliders", &es->show_colliders);
				if (es->show_colliders)
				{
					// @TODO: Entity loop
					for (int32 i = 0; i < ArrayCount(gs->entites); i++)
					{
						Entity* entity = &gs->entites[i];
						if (IsValidEntity(gs, entity) && entity->active)
						{
							OBB collider = GetEntityBoxCollider(entity);
							DEBUGDrawOBB(collider);
						}
					}
				}

				ImGui::End();
			}

			ImGuiIO& io = ImGui::GetIO();
			if (!io.WantCaptureMouse)
			{
				if (IsKeyJustDown(input, mb1))
				{
					es->selected_entity = SelectEntity(es, gs, rs, as, ws, input);
					es->selectedCollider = nullptr;
				}
			}

			if (es->selected_entity)
			{
				OperateGizmo(es, gs, ws, input);

				if (input->ctrl && IsKeyJustDown(input, d))
				{
					Entity* entity = CreateEntity(gs);

					EntityId id = entity->id;
					*entity = *es->selected_entity;
					entity->id = id;

					entity->transform.position += entity->transform.GetBasis().right * 0.5f;
					es->selected_entity = entity;
				}

				if (IsKeyJustDown(input, del))
				{
					RemoveEntity(gs, es->selected_entity);
					es->selected_entity = nullptr;
				}
			}

			ProcessUndoSystems(gs, es, input);


			RenderTarget* rt = &rs->gbufferRenderTarget;
			float size = 512;
			//ImGui::Image(rs->bloomRenderTarget.colourTexture0.view, ImVec2(size, size));
			//ImGui::Image(rs->forwardPassRenderTarget.colourTexture0.view, ImVec2(size, size));
			//ImGui::Image(rs->shadowCube.faceViews[0], ImVec2(size, size));
			//ImGui::Image(rs->shadow_atlas.colourTexture0.view, ImVec2(size, size));
			//ImGui::Image(rt->colourTexture2.view, ImVec2(size, size));
			//ImGui::Image(rs->tempShadowbuffer.view, ImVec2(size, size));
			//ImGui::Image(rs->shadowBuffer.view, ImVec2(size, size));
		}

		ClearRenderGroup(&gs->entityRenderGroup);
		ConstructRenderGroup(gs, &gs->entityRenderGroup);
		gs->entityRenderGroup.mainCamera = es->camera;
		gs->entityRenderGroup.player = gs->player;
	}
}
#endif