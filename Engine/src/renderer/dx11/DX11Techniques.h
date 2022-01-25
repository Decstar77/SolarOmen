#pragma once
#include "DX11Types.h"

namespace sol
{
	class RenderTechnique
	{
	public:
		static CubeMapInstance ConvertEqiTextureToCubeMap(RenderState* rs, uint32 resolution, const StaticTexture& eqi);
		static CubeMapInstance ConvoluteCubeMap(uint32 resolution, const CubeMapInstance& cube);
	};

}