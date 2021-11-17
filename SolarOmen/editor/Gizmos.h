#pragma once
#include "core/SolarCore.h"
#include "SolarOmen.h"

namespace cm
{
	class GizmoOperation
	{
	public:
		enum class Value : uint32
		{
			TRANSLATE = 0,
			ROTATE,
			SCALE,
			COUNT,
		};

	public:
		GizmoOperation();
		GizmoOperation(Value v);
		CString ToString() const;
		static GizmoOperation ValueOf(const uint32& v);
		static GizmoOperation ValueOf(const CString& str);
		inline bool operator==(const GizmoOperation& rhs) const { return this->value == rhs.value; }
		inline bool operator!=(const GizmoOperation& rhs) const { return this->value != rhs.value; }
		inline operator uint32() const { return (uint32)value; }

	private:
		Value value;
		static inline CString __STRINGS__[] =
		{
			"TRANSLATE",
			"ROTATE",
			"SCALE"
		};
	};

	class GizmoMode
	{
	public:
		enum class Value : uint32
		{
			LOCAL = 0,
			WORLD,
			COUNT
		};

	public:
		GizmoMode();
		GizmoMode(Value v);
		CString ToString() const;
		static GizmoMode ValueOf(const uint32& v);
		static GizmoMode ValueOf(const CString& str);
		inline bool operator==(const GizmoMode& rhs) const { return this->value == rhs.value; }
		inline bool operator!=(const GizmoMode& rhs) const { return this->value != rhs.value; }
		inline operator uint32() const { return (uint32)value; }

	private:
		Value value;

		static inline CString __STRINGS__[] =
		{
			"LOCAL",
			"WORLD"
		};
	};

	class Gizmo
	{
	public:
		GizmoOperation operation;
		GizmoMode mode;

		bool snapping;
		Vec3f snapAmount;

		void Operate(const Camera& camera, Entity* entity);

	private:
		void CheckOperationAndMode();
		void CheckSnapping();
	};

}


