#include "SolarAssets.h"
#include "core/SolarPlatform.h"

#if USE_RAW_ASSETS
#include "serialization/RawLoaders.h"
#else 
#include "serialization/PackedLoaders.h"
#endif

namespace cm
{
	void InitializeAssets(AssetState* as)
	{
		LoadAllShaders(as);
		LoadAllModels(as);
		LoadAllTextures(as);
		LoadAllFonts(as);
		LoadAllAudio(as);
	}
}
