#include "SolarAssets.h"
#include "core/SolarPlatform.h"

#if USE_RAW_ASSETS
#include "serialization/RawLoaders.h"
#else 
#include "serialization/PackedLoaders.h"
#endif

namespace cm
{
	void AssetState::Initialize(AssetState* as)
	{
		assetState = as;
		LoadAllShaders(as);
		LoadAllModels(as);
		LoadAllTextures(as);
		LoadAllFonts(as);
		LoadAllAudio(as);
		LoadAllRacingTracks(as);
	}
}
