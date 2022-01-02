#include "DX11Texture.h"
#include "DX11Renderer.h"

namespace cm
{
	SamplerInstance SamplerInstance::Create(TextureFilterMode filter, TextureWrapMode wrap)
	{
		GetRenderState();

		SamplerInstance result = {};
		result.filter = filter;
		result.wrap = wrap;

		D3D11_SAMPLER_DESC sampDesc = {};
		sampDesc.Filter = GetTextureFilterModeToD3D(filter);
		sampDesc.AddressU = GetTextureWrapModeToD3D(wrap);
		sampDesc.AddressV = GetTextureWrapModeToD3D(wrap);
		sampDesc.AddressW = GetTextureWrapModeToD3D(wrap);

		DXCHECK(rs->device->CreateSamplerState(&sampDesc, &result.sampler));

		return result;
	}

	SamplerInstance SamplerInstance::CreateShadowPFC()
	{
		GetRenderState();

		SamplerInstance result = {};
		D3D11_SAMPLER_DESC sampDesc = {};

		sampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampDesc.MipLODBias = 0.0f;
		sampDesc.MaxAnisotropy = 1;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
		sampDesc.BorderColor[0] = sampDesc.BorderColor[1] = sampDesc.BorderColor[2] = sampDesc.BorderColor[3] = 0;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

		DXCHECK(rs->device->CreateSamplerState(&sampDesc, &result.sampler));

		return result;
	}

	TextureInstance TextureInstance::Create(const FontCharacter& fontChar)
	{
		TextureInstance result = {};
		result.width = fontChar.size.x;
		result.height = fontChar.size.y;
		result.format = TextureFormat::Value::R8_BYTE;
		result.cpuFlags = ResourceCPUFlags::NONE;
		result.usage[0] = TextureUsage::SHADER_RESOURCE;

		int32 bind_flags = 0;
		for (int32 i = 0; i < ArrayCount(result.usage); i++)
		{
			result.usage[i] = result.usage[i];
			bind_flags |= GetTextureUsageToD3DBindFlags(result.usage[i]);
		}

		GetRenderState();

		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = result.width;
		desc.Height = result.height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = result.cpuFlags == ResourceCPUFlags::NONE ? D3D11_USAGE_DEFAULT : D3D11_USAGE_STAGING;
		desc.BindFlags = bind_flags;
		desc.CPUAccessFlags = GetCPUFlagsToD3DFlags(result.cpuFlags);
		desc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA sd = {};
		sd.pSysMem = fontChar.data.data;
		sd.SysMemPitch = result.width * GetTextureFormatElementSizeBytes(result.format) * GetTextureFormatElementCount(result.format);
		DXCHECK(rs->device->CreateTexture2D(&desc, &sd, &result.texture));

		D3D11_SHADER_RESOURCE_VIEW_DESC view_desc = {};
		view_desc.Format = desc.Format;
		view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		view_desc.Texture2D.MostDetailedMip = 0;
		view_desc.Texture2D.MipLevels = 1;
		DXCHECK(rs->device->CreateShaderResourceView(result.texture, &view_desc, &result.shaderView));

		return result;
	}

	TextureInstance TextureInstance::Create(const TextureAsset& textureAsset)
	{
		Assert(!textureAsset.mips, "Todo mips");
		Assert(textureAsset.width > 0, "Width is zero");
		Assert(textureAsset.height > 0, "Width is zero");

		GetRenderState();

		TextureInstance result = {};
		result.id = textureAsset.id;
		result.width = textureAsset.width;
		result.height = textureAsset.height;
		result.format = textureAsset.format;
		result.cpuFlags = textureAsset.cpuFlags;

		int32 bind_flags = 0;
		for (int32 i = 0; i < ArrayCount(textureAsset.usage); i++)
		{
			result.usage[i] = textureAsset.usage[i];
			bind_flags |= GetTextureUsageToD3DBindFlags(textureAsset.usage[i]);
		}

		// @NOTE: create the raw texture
		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = textureAsset.width;
		desc.Height = textureAsset.height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = GetTextureFormatToD3D(textureAsset.format);
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = textureAsset.cpuFlags == ResourceCPUFlags::NONE ? D3D11_USAGE_DEFAULT : D3D11_USAGE_STAGING;
		desc.BindFlags = bind_flags;
		desc.CPUAccessFlags = GetCPUFlagsToD3DFlags(textureAsset.cpuFlags);
		desc.MiscFlags = 0;

		if (textureAsset.pixels)
		{
			D3D11_SUBRESOURCE_DATA sd = {};
			sd.pSysMem = textureAsset.pixels;
			sd.SysMemPitch = textureAsset.width * GetTextureFormatElementSizeBytes(textureAsset.format) * GetTextureFormatElementCount(textureAsset.format);
			DXCHECK(rs->device->CreateTexture2D(&desc, &sd, &result.texture));
		}
		else
		{
			DXCHECK(rs->device->CreateTexture2D(&desc, NULL, &result.texture));
		}

		Assert(result.texture, "Could not create texture");

		if (result.texture && (bind_flags & D3D11_BIND_SHADER_RESOURCE))
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC view_desc = {};
			if (textureAsset.format == TextureFormat::Value::R32_TYPELESS)
			{
				LOG("Warning making r32typeless format into a r32 float for shader resource");
				view_desc.Format = DXGI_FORMAT_R32_FLOAT;
			}
			else
			{
				view_desc.Format = desc.Format;
			}
			view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			view_desc.Texture2D.MostDetailedMip = 0;
			view_desc.Texture2D.MipLevels = 1;

			DXCHECK(rs->device->CreateShaderResourceView(result.texture, &view_desc, &result.shaderView));
		}

		if (result.texture && (bind_flags & D3D11_BIND_UNORDERED_ACCESS))
		{
			D3D11_UNORDERED_ACCESS_VIEW_DESC uav = {};
			uav.Format = desc.Format;
			uav.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
			uav.Texture2D.MipSlice = 0;

			DXINFO(rs->device->CreateUnorderedAccessView(result.texture, &uav, &result.uavView));
		}

		if (result.texture && (bind_flags & D3D11_BIND_DEPTH_STENCIL))
		{
			D3D11_DEPTH_STENCIL_VIEW_DESC depth_view_dsc = {};
			if (textureAsset.format == TextureFormat::Value::R32_TYPELESS)
			{
				LOG("Warning making r32typeless format into a d32 float for depth-stencil buffer");
				depth_view_dsc.Format = DXGI_FORMAT_D32_FLOAT;
			}
			else
			{
				depth_view_dsc.Format = desc.Format;
			}
			depth_view_dsc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			depth_view_dsc.Texture2D.MipSlice = 0;
			DXCHECK(rs->device->CreateDepthStencilView(result.texture,
				&depth_view_dsc, &result.depthView));
		}

		if (result.texture && (bind_flags & D3D11_BIND_RENDER_TARGET))
		{
			D3D11_RENDER_TARGET_VIEW_DESC render_target_desc = {};
			render_target_desc.Format = desc.Format;
			render_target_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			render_target_desc.Texture2D.MipSlice = 0;

			DXCHECK(rs->device->CreateRenderTargetView(result.texture,
				&render_target_desc, &result.renderView));
		}

		return result;
	}

	CubeMapInstance CubeMapInstance::Create(uint32 resolution)
	{
		GetRenderState();

		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = resolution;
		desc.Height = resolution;
		desc.MipLevels = 1;
		desc.ArraySize = 6;
		desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

		CubeMapInstance result = {};
		DXCHECK(rs->device->CreateTexture2D(&desc, NULL, &result.texture));

		D3D11_SHADER_RESOURCE_VIEW_DESC view_desc = {};
		view_desc.Format = desc.Format;
		view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		view_desc.Texture2D.MostDetailedMip = 0;
		view_desc.Texture2D.MipLevels = 1;

		DXCHECK(rs->device->CreateShaderResourceView(result.texture, &view_desc, &result.shaderView));

		D3D11_RENDER_TARGET_VIEW_DESC render_target_desc = {};
		render_target_desc.Format = desc.Format;
		render_target_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		render_target_desc.Texture2DArray.MipSlice = 0;
		render_target_desc.Texture2DArray.ArraySize = 1;

		for (int32 i = 0; i < 6; i++)
		{
			render_target_desc.Texture2DArray.FirstArraySlice = i;
			DXCHECK(rs->device->CreateRenderTargetView(result.texture,
				&render_target_desc, &result.renderFaces[i]));
		}

		result.renderFaces.count = 6;

		return result;
	}
}