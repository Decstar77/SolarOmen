#pragma once

#include "DX11Header.h"

namespace cm
{
	struct StaticMesh
	{
		AssetId id;
		uint32 strideBytes; // @NOTE: Used for rendering
		uint32 indexCount; // @NOTE: Used for rendering
		ID3D11Buffer* vertexBuffer;
		ID3D11Buffer* indexBuffer;

		static StaticMesh Create(const ModelAsset& modelAsset);
		static StaticMesh CreateScreenSpaceQuad();
	};



}
