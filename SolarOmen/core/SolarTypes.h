#pragma once
#include "Defines.h"
#include "SolarString.h"
#include "SolarContainers.h"

namespace cm
{
	struct EntityRenderGroup;

	class Room;
	class Entity;

	struct EntityId
	{
		int32 index;
		int32 generation;

		Entity* Get() const;
		inline CString ToString() const { return CString("Index;").Add(index).Add(":Gen;").Add(generation); };

		inline bool operator==(const EntityId& rhs) const
		{
			return this->index == rhs.index && this->generation == rhs.generation;
		}

		inline bool operator!=(const EntityId& rhs) const
		{
			return this->index != rhs.index || this->generation != rhs.generation;
		}
	};

	typedef uint64 AssetId;
#define INVALID_ASSET_ID 0
}
