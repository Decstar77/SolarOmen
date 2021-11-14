#include "Gizmos.h"

#include "SolarEditor.h"

#include "../vendor/imgui/imgui.h"
#include "../vendor/imguizmo/ImGuizmo.h"

namespace cm
{
	GizmoOperation::GizmoOperation()
	{
		value = Value::TRANSLATE;
	}

	GizmoOperation::GizmoOperation(Value v)
	{
		this->value = v;
	}

	CString GizmoOperation::ToString() const
	{
		CString copy = __STRINGS__[(uint32)value];

		return copy;
	}

	GizmoOperation GizmoOperation::ValueOf(const uint32& v)
	{
		Assert(v < (uint32)Value::COUNT, "Invalid model id");
		return (GizmoOperation::Value)v;
	}

	GizmoOperation GizmoOperation::ValueOf(const CString& str)
	{
		uint32 count = (uint32)Value::COUNT;
		for (uint32 i = 0; i < count; i++)
		{
			if (str == __STRINGS__[i])
			{
				return ValueOf(i);
			}
		}

		return Value::TRANSLATE;
	}

	GizmoMode::GizmoMode()
	{
		value = Value::WORLD;
	}

	GizmoMode::GizmoMode(Value v)
	{
		this->value = v;
	}

	CString GizmoMode::ToString() const
	{
		CString copy = __STRINGS__[(uint32)value];

		return copy;
	}

	GizmoMode GizmoMode::ValueOf(const uint32& v)
	{
		Assert(v < (uint32)Value::COUNT, "Invalid model id");
		return (GizmoMode::Value)v;
	}

	GizmoMode GizmoMode::ValueOf(const CString& str)
	{
		uint32 count = (uint32)Value::COUNT;
		for (uint32 i = 0; i < count; i++)
		{
			if (str == __STRINGS__[i])
			{
				return ValueOf(i);
			}
		}

		return Value::WORLD;
	}

	void Gizmo::Operate(const Camera& camera, Entity* entity, Input* input)
	{
		ImGuizmo::Enable(true);
		ImGuiIO& io = ImGui::GetIO();
		ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

		CheckOperationAndMode(input);
		CheckSnapping(input);

		Mat4f view = camera.GetViewMatrix();
		Mat4f proj = camera.GetProjectionMatrix();

		{
			ImGuizmo::OPERATION op = (ImGuizmo::OPERATION)((uint32)(this->operation));
			ImGuizmo::MODE md = (ImGuizmo::MODE)((uint32)(this->mode));

			Mat4f mat = entity->GetWorldTransform().CalculateTransformMatrix();

			ImGuizmo::Manipulate(view.ptr, proj.ptr, op, md, mat.ptr, nullptr, snapping ? (snapAmount.ptr) : nullptr);

			if (Entity* parent = entity->GetParent())
			{
				Mat4 invMP = Inverse(parent->GetWorldTransform().CalculateTransformMatrix());
				mat = mat * invMP;
			}

			Vec3f pos;
			Vec3f euler;
			Vec3f scale;
			ImGuizmo::DecomposeMatrixToComponents(mat.ptr, pos.ptr, euler.ptr, scale.ptr);
			Quatf qori = EulerToQuat(euler);

			// @NOTE: These equals are done to prevent floating point errors that trick 
			//		: the undo system for the game state to think that a change has occurred

			if (!Equal(pos, entity->transform.position))
			{
				entity->transform.position = pos;
			}

			if (!Equal(qori, entity->transform.orientation))
			{
				entity->transform.orientation = qori;
			}

			if (!Equal(scale, entity->transform.scale))
			{
				entity->transform.scale = scale;
			}
		}
	}

	void Gizmo::CheckOperationAndMode(Input* input)
	{
		if (IsKeyJustDown(input, e) && !input->mb1)
		{
			this->operation = GizmoOperation::Value::SCALE;
		}
		if (IsKeyJustDown(input, r) && !input->mb1)
		{
			this->operation = GizmoOperation::Value::ROTATE;
		}
		if (IsKeyJustDown(input, t) && !input->mb1)
		{
			this->operation = GizmoOperation::Value::TRANSLATE;
		}

		if (IsKeyJustDown(input, tlda) && !input->mb1)
		{
			this->mode = (mode == GizmoMode::Value::LOCAL) ? GizmoMode::Value::WORLD : GizmoMode::Value::LOCAL;
		}
	}

	void Gizmo::CheckSnapping(Input* input)
	{
		Vec3f snapAmount = Vec3f(VOXEL_IMPORT_SCALE);
		if (this->operation == GizmoOperation::Value::ROTATE)
		{
			snapAmount = Vec3f(15);
		}
		if (input->ctrl)
		{
			snapping = true;
		}
		else
		{
			snapping = false;
		}
	}
}

