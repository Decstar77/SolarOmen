#pragma once
#include "../SolarAssets.h"
#include "core/SolarPlatform.h"

namespace cm
{
#if USE_RAW_ASSETS
	void LoadAllShaders(AssetState* as);
	void LoadAllModels(AssetState* as);
	void LoadAllTextures(AssetState* as);
	void LoadAllFonts(AssetState* as);
	void LoadAllAudio(AssetState* as);
	void LoadAllRacingTracks(AssetState* as);
#endif
}
