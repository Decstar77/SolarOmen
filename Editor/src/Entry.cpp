
#include "Core.h"
#include "ImGuiLayer.h"

namespace sol
{
	static EditorState* es = nullptr;

	bool8 OnWindowResizeCallback(uint16 eventCode, void* sender, void* listener, EventContext context)
	{
		uint32 w = Platform::GetWindowWidth();
		uint32 h = Platform::GetWindowHeight();
		es->camera.aspect = (real32)w / (real32)h;

		return 0;
	}

	static bool8 GameInitialze(Game* game)
	{
		if (InitialzieImGui())
		{
			uint32 w = Platform::GetWindowWidth();
			uint32 h = Platform::GetWindowHeight();

			es->camera.transform = Transform();
			es->camera.transform.position.z = -3;
			es->camera.yaw = 90.0f;
			es->camera.far_ = 250.0f;
			es->camera.near_ = 0.3f;
			es->camera.yfov = 45.0f;
			es->camera.aspect = (real32)w / (real32)h;

			EventSystem::Register((uint16)EngineEvent::Value::WINDOW_RESIZED, nullptr, OnWindowResizeCallback);

			es->room.Initliaze();
			es->selectedEntity = es->room.CreateEntity("Test");

			MaterialComponent* materailComponent = es->selectedEntity.GetMaterialomponent();
			//materailComponent->material.albedoId = Resources::GetTextureResource("BoomBox_baseColor")->id;
			//materailComponent->material.modelId = Resources::GetModelResource("BoomBox")->id;


			return true;
		}

		return false;
	}

	bool OperateCamera(Camera* camera, real32 dtime)
	{
		Input* input = Input::Get();
		bool operating = false;
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
			if (input->w) { camera->transform.position += (basis.forward * move_speed * dtime); }
			if (input->s) { camera->transform.position += (-1.0f * basis.forward * move_speed * dtime); }
			if (input->a) { camera->transform.position += (-1.0f * basis.right * move_speed * dtime); }
			if (input->d) { camera->transform.position += (basis.right * move_speed * dtime); }
			if (input->q) { camera->transform.position += (Vec3f(0, 1, 0) * move_speed * dtime); }
			if (input->e) { camera->transform.position += (Vec3f(0, -1, 0) * move_speed * dtime); }

			operating = true;
		}
		else
		{
			input->mouse_locked = false;
		}

		return operating;
	}

	static bool8 GameUpdate(Game* game, RenderPacket* renderPacket, real32 dt)
	{
		UpdateImGui(es, dt);

		Input* input = Input::Get();
		if (IsKeyJustDown(input, escape))
		{
			return false;
		}

		if (IsKeyJustDown(input, f5))
		{
			es->isLightMapping = !es->isLightMapping;

			if (es->isLightMapping)
			{
				int32 w = (int32)Platform::GetWindowWidth();
				int32 h = (int32)Platform::GetWindowHeight();
				TextureFormat format = TextureFormat::Value::R32G32B32A32_FLOAT;
				es->raytracedTexture = Renderer::CreateTexture(w, h, format, ResourceCPUFlags::Value::WRITE);
				es->raytracePixels.resize(w * h * format.GetPitchBytes());
			}
			if (!es->isLightMapping)
			{
				Renderer::DestroyTexture(&es->raytracedTexture);
			}
		}

		if (!es->isLightMapping)
		{
			OperateCamera(&es->camera, dt);

			renderPacket->cameraPos = es->camera.transform.position;
			renderPacket->viewMatrix = es->camera.GetViewMatrix();
			renderPacket->projectionMatrix = es->camera.GetProjectionMatrix();

			if (es->selectedEntity.IsValid())
			{
				RenderEntry entry = {};
				entry.worldTransform = es->selectedEntity.GetWorldTransform();
				entry.material = es->selectedEntity.GetMaterialomponent()->material;

				renderPacket->renderEntries.Add(entry);
			}
		}
		else
		{
			es->raytraceUpdateCounter++;
			if (es->raytraceUpdateCounter == 30)
			{
				es->raytraceUpdateCounter = 0;
				for (Vec4f& pixel : es->raytracePixels)
				{
					pixel.r = 1.0f;
				}


				Renderer::UpdateWholeTexture(es->raytracedTexture, es->raytracePixels.data());
			}


			renderPacket->viewMatrix = Mat4f(1);
			renderPacket->projectionMatrix = Mat4f(1);

			RenderEntry entry = {};
			entry.material.modelId.number = 1;
			entry.material.albedoTexture = es->raytracedTexture;

			renderPacket->renderEntries.Add(entry);
		}

		return true;
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

		es = GameMemory::PushPermanentClass<EditorState>();

		return true;
	}

}