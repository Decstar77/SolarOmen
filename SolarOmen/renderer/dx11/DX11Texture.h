#pragma once

#include "DX11Header.h"

namespace cm
{
	struct TextureInstance
	{
		AssetId id;

		int32 width;
		int32 height;
		TextureFormat format;
		TextureUsage usage[4];
		TextureCPUFlags cpuFlags;

		ID3D11Texture2D* texture;

		ID3D11ShaderResourceView* shaderView;
		ID3D11UnorderedAccessView* uavView;
		ID3D11DepthStencilView* depthView;
		ID3D11RenderTargetView* renderView;

		static TextureInstance Create(const FontCharacter& fontChar);
		static TextureInstance Create(const TextureAsset& textureAsset);
	};

	struct SamplerInstance
	{
		TextureFilterMode filter;
		TextureWrapMode wrap;

		ID3D11SamplerState* sampler;

		static SamplerInstance Create(TextureFilterMode filter, TextureWrapMode wrap);
		static SamplerInstance CreateShadowPFC();
	};
}
