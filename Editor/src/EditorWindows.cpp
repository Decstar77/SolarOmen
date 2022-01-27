#include "EditorWindows.h"



namespace sol
{
	EditorWindowList::EditorWindowList() : EditorWindow("WINDOW LIST", true)
	{
	}

	bool8 EditorWindowList::Show(EditorState* es)
	{
		for (uint64 i = 0; i < windows.size(); i++)
		{
			auto window = windows.at(i);

			if (window->ShouldShow())
			{
				bool8 remove = window->Show(es);
				if (remove)
				{
					windows.erase(windows.begin() + i);
					i--;
				}
			}
		}

		return windows.empty();
	}

	bool8 EditorWindowList::Add(std::shared_ptr<EditorWindow> window)
	{
		for (const auto& w : windows)
		{
			if (w->GetName() == window->GetName())
			{
				return false;
			}
		}

		windows.push_back(window);
		return true;
	}

	TextureMetaFileWindow::TextureMetaFileWindow(const String& name) : EditorWindow(name, true)
	{
		FileProcessor fileProcessor = {};
		MetaProcessor metaProcessor = {};
		metaProcessor.LoadAllMetaFiles(fileProcessor.GetFilePaths(ASSET_PATH, "slo"));
		path = metaProcessor.Find(name);
		file = metaProcessor.ParseTextureMetaFile(path);
		if (!file.id.IsValid())
		{
			show = false;
			SOLERROR("Could not edit texture meta file");
		}
	}

	bool8 TextureMetaFileWindow::Show(EditorState* es)
	{
		String windowName = "MetaData";
		windowName.Add(name);
		if (ImGui::Begin(windowName.GetCStr(), &show))
		{
			ImGui::Text("Id: %llu", file.id.number);
			ImGui::Text("Format: %s", file.format.ToString().GetCStr());
			ImGui::Checkbox("Generate Mip maps", &file.mips);
			ImGui::Checkbox("Is Skybox", &file.isSkybox);
			ImGui::Checkbox("Is Normal Map", &file.isNormalMap);

			if (ImGui::Button("Save"))
			{
				MetaProcessor metaProcessor = {};
				metaProcessor.SaveMetaData(path, file);
			}


			ImGui::End();
		}

		return !show;
	}


	bool8 EditorPerformanceWindow::Show(EditorState* es)
	{
		if (ImGui::Begin(GetName().GetCStr(), &show))
		{
			ImGui::Text("Permanent Memory Used: %llu mbs", GameMemory::GetTheAmountOfPermanentMemoryUsed() / (1024 * 1024));
			ImGui::Text("Transient Memory Used: %llu mbs", GameMemory::GetTheAmountOfTransientMemoryUsed() / (1024 * 1024));

			ImGui::Text("Permanent Memory Allocated: %llu mbs", GameMemory::GetTheTotalAmountOfPermanentMemoryAllocated() / (1024 * 1024));
			ImGui::Text("Transient Memory Allocated: %llu mbs", GameMemory::GetTheTotalAmountOfTransientMemoryAllocated() / (1024 * 1024));

			ImGui::Separator();

			for (int32 i = 0; i < ArrayCount(frameTimes) - 1; i++)
			{
				frameTimes[i] = frameTimes[i + 1];
			}

			real32 dt = Application::GetDeltaTime();
			frameTimes[ArrayCount(frameTimes) - 1] = dt * 1000;

			minTime = Min(minTime, dt);
			maxTime = Max(maxTime, dt);

			ImGui::Text("Min %f", minTime * 1000.0f);
			ImGui::SameLine();
			ImGui::Text("Max %f", maxTime * 1000.0f);
			ImGui::SameLine();
			if (ImGui::Button("Reset")) { minTime = REAL_MAX; maxTime = REAL_MIN; }
			ImGui::PlotLines("Frame time", frameTimes, ArrayCount(frameTimes), 0, 0, 0, 30, ImVec2(128, 128), 4);


			ImGui::End();
		}

		return !show;
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

	static void DoEntityTreeDisplay(EditorState* es, Entity entity)
	{
		ImGui::PushID(entity.GetId().index);
		bool8 open = ImGui::TreeNodeEx(entity.GetName().GetCStr(),
			ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_DefaultOpen |
			ImGuiTreeNodeFlags_OpenOnArrow |
			ImGuiTreeNodeFlags_SpanAvailWidth, entity.GetName().GetCStr());
		ImGui::PopID();

		const char* dragDropType = "SCENE ENTITY";

		if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsItemHovered())
		{

		}

		if (ImGui::BeginDragDropSource())
		{
			ImGui::SetDragDropPayload(dragDropType, &entity, sizeof(entity));
			ImGui::Text(entity.GetName().GetCStr());
			ImGui::EndDragDropSource();
		}

		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* load = ImGui::AcceptDragDropPayload(dragDropType);
			if (load)
			{
				Assert(sizeof(Entity) == load->DataSize, "DoEntityTreeDisplay not an entity ?");
				Entity dragEntity = *(Entity*)load->Data;

				if (dragEntity != entity)
				{
					dragEntity.SetParent(&entity);
				}
			}

			ImGui::EndDragDropTarget();
		}

		if (open)
		{
			for (Entity* child = entity.GetFirstChild(); child != nullptr; child = child->GetSiblingAhead())
			{
				DoEntityTreeDisplay(es, *child);
			}

			ImGui::TreePop();
		}
	}

	bool8 EditorRoomSettingsWindow::Show(EditorState* es)
	{
		if (ImGui::Begin(GetName().GetCStr(), &show))
		{
			ImGui::InputText("Name", room->name.GetCStr(), room->name.CAPCITY);
			room->name.CalculateLength();

			room->skyboxId = ComboBoxOfAsset("Skybox", Resources::GetAllTextureResources(), room->skyboxId);

			if (ImGui::CollapsingHeader("Entities"))
			{
				room->BeginEntityLoop();
				while (Entity entity = room->GetNextEntity())
				{
					Entity* parent = entity.GetParent();
					if (!parent)
					{
						DoEntityTreeDisplay(es, entity);
					}
				}
			}

			ImGui::End();
		}
		return !show;
	}

}