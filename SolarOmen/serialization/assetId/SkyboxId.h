#pragma once
#include "core/SolarCore.h"
namespace cm
{
	enum class SkyboxId
	{
		INVALID = 0,
		// @NOTE: External
		NEWPORT_LOFT,
		STUDIO_SMALL_08_4K,
		COUNT,
	};


	inline static const CString __SKYBOX_STRINGS__[] = {
		"INVALID",
		"NEWPORT_LOFT",
		"STUDIO_SMALL_08_4K",
		"COUNT"
	};

	inline SkyboxId GetSkyboxIdFromString(CString str)
	{
		int32 count = (int32)SkyboxId::COUNT;
		for (int32 i = 0; i < count; i++)
		{
			if (str == __SKYBOX_STRINGS__[i])
			{
				return (SkyboxId)(i);
			}
		}

		return SkyboxId::INVALID;
	}

}