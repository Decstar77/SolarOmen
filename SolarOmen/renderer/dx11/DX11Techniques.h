#pragma once

#include "DX11Header.h"
#include "DX11Texture.h"
#include "DX11StaticMesh.h"
#include "DX11Shader.h"
#include "DX11Commands.h"

namespace cm
{
	CubeMapInstance ConvertEqiTextureToCubeMap(uint32 resolution, const TextureInstance& eqi);
	CubeMapInstance ConvoluteCubeMap(uint32 resolution, const CubeMapInstance& cube);
}
