#pragma once

#include "core/SolarCore.h"

namespace cm
{
	struct EntityId
	{
		int32 index;
		int32 generation;

		class Entity* Get() const;
		CString ToString();

		inline bool operator==(const EntityId& rhs) const
		{
			return this->index == rhs.index && this->generation == rhs.generation;
		}

		inline bool operator!=(const EntityId& rhs) const
		{
			return this->index != rhs.index || this->generation != rhs.generation;
		}
	};

	enum class PlayerNumber
	{
		NONE = 0,
		ONE,
		TWO,
	};

	inline PlayerNumber  GetOppositePlayer(const PlayerNumber& playerNumber)
	{
		if (playerNumber == PlayerNumber::ONE)
			return PlayerNumber::TWO;
		if (playerNumber == PlayerNumber::TWO)
			return PlayerNumber::ONE;
		return PlayerNumber::NONE;
	}
}
