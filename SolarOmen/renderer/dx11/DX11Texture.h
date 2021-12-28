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

	struct CubeMapInstance
	{
		AssetId id;

		ID3D11Texture2D* texture = nullptr;
		ID3D11ShaderResourceView* shaderView = nullptr;

		FixedArray<ID3D11RenderTargetView*, 6> renderFaces;

		static CubeMapInstance Create(uint32 resolution);

		//void Bind(RenderState* rs, ShaderStage shaderStage, int32 register_);
		//void Unbind(RenderState* rs);
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
