#include "DX11Types.h"
#include "core/SolarLogging.h"
#if SOLAR_PLATFORM_WINDOWS && USE_DIRECTX11
namespace sol
{
	void StaticTexture::Release(StaticTexture* texture)
	{
		DXRELEASE(texture->shaderView);
		DXRELEASE(texture->uavView);
		DXRELEASE(texture->depthView);
		DXRELEASE(texture->renderView);
		DXRELEASE(texture->texture);

		GameMemory::ZeroStruct(texture);
	}

	StaticTexture StaticTexture::Create(int32 width, int32 height, TextureFormat format, void* pixels, bool8 mips, BindUsage* usage, ResourceCPUFlags cpuFlags)
	{
		StaticTexture result = {};
		result.width = width;
		result.height = height;
		result.format = format;
		result.cpuFlags = cpuFlags;

		int32 bind_flags = 0;
		for (int32 i = 0; i < ArrayCount(result.usage); i++)
		{
			result.usage[i] = usage[i];
			bind_flags |= GetTextureUsageToD3DBindFlags(usage[i]);
		}

		DeviceContext dc = GetDeviceContext();

		// @NOTE: create the raw texture
		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = GetTextureFormatToD3D(format);
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = cpuFlags == ResourceCPUFlags::Value::NONE ? D3D11_USAGE_DEFAULT : D3D11_USAGE_STAGING;
		desc.BindFlags = bind_flags;
		desc.CPUAccessFlags = GetCPUFlagsToD3DFlags(cpuFlags);
		desc.MiscFlags = 0;

		if (pixels)
		{
			D3D11_SUBRESOURCE_DATA sd = {};
			sd.pSysMem = pixels;
			sd.SysMemPitch = width * GetTextureFormatElementSizeBytes(format) * GetTextureFormatElementCount(format);
			DXCHECK(dc.device->CreateTexture2D(&desc, &sd, &result.texture));

		}
		else
		{
			DXCHECK(dc.device->CreateTexture2D(&desc, NULL, &result.texture));
		}

		SOLTRACE(String("Created textured: ").Add((width * height *
			GetTextureFormatElementSizeBytes(format) * GetTextureFormatElementCount(format) / 1024)).Add(" kbs").GetCStr());

		Assert(result.texture, "Could not create texture");

		if (result.texture && (bind_flags & D3D11_BIND_SHADER_RESOURCE))
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC view_desc = {};
			if (format == TextureFormat::Value::R32_TYPELESS)
			{
				SOLWARN("Making r32typeless format into a r32 float for shader resource");
				view_desc.Format = DXGI_FORMAT_R32_FLOAT;
			}
			else
			{
				view_desc.Format = desc.Format;
			}
			view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			view_desc.Texture2D.MostDetailedMip = 0;
			view_desc.Texture2D.MipLevels = 1;

			DXCHECK(dc.device->CreateShaderResourceView(result.texture, &view_desc, &result.shaderView));
		}

		if (result.texture && (bind_flags & D3D11_BIND_UNORDERED_ACCESS))
		{
			D3D11_UNORDERED_ACCESS_VIEW_DESC uav = {};
			uav.Format = desc.Format;
			uav.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
			uav.Texture2D.MipSlice = 0;

			DXINFO(dc.device->CreateUnorderedAccessView(result.texture, &uav, &result.uavView));
		}

		if (result.texture && (bind_flags & D3D11_BIND_DEPTH_STENCIL))
		{
			D3D11_DEPTH_STENCIL_VIEW_DESC depth_view_dsc = {};
			if (format == TextureFormat::Value::R32_TYPELESS)
			{
				SOLWARN("Making r32typeless format into a d32 float for depth-stencil buffer");
				depth_view_dsc.Format = DXGI_FORMAT_D32_FLOAT;
			}
			else
			{
				depth_view_dsc.Format = desc.Format;
			}
			depth_view_dsc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			depth_view_dsc.Texture2D.MipSlice = 0;
			DXCHECK(dc.device->CreateDepthStencilView(result.texture,
				&depth_view_dsc, &result.depthView));
		}

		if (result.texture && (bind_flags & D3D11_BIND_RENDER_TARGET))
		{
			D3D11_RENDER_TARGET_VIEW_DESC render_target_desc = {};
			render_target_desc.Format = desc.Format;
			render_target_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			render_target_desc.Texture2D.MipSlice = 0;

			DXCHECK(dc.device->CreateRenderTargetView(result.texture,
				&render_target_desc, &result.renderView));
		}

		return result;
	}

	StaticTexture StaticTexture::Create(TextureResource* textureResource)
	{
		StaticTexture texture = StaticTexture::Create(
			textureResource->width, textureResource->height, textureResource->format,
			(void*)textureResource->pixels.data, false, textureResource->usage, textureResource->cpuFlags
		);

		return texture;
	}

	void SamplerState::Release(SamplerState* sampler)
	{
		DXRELEASE(sampler->sampler);
		GameMemory::ZeroStruct(sampler);
	}

	SamplerState SamplerState::Create(TextureFilterMode filter, TextureWrapMode wrap)
	{
		DeviceContext dc = GetDeviceContext();

		SamplerState result = {};
		result.filter = filter;
		result.wrap = wrap;

		D3D11_SAMPLER_DESC sampDesc = {};
		sampDesc.Filter = GetTextureFilterModeToD3D(filter);
		sampDesc.AddressU = GetTextureWrapModeToD3D(wrap);
		sampDesc.AddressV = GetTextureWrapModeToD3D(wrap);
		sampDesc.AddressW = GetTextureWrapModeToD3D(wrap);

		DXCHECK(dc.device->CreateSamplerState(&sampDesc, &result.sampler));

		return result;
	}

	SamplerState SamplerState::CreateShadowPFC()
	{
		DeviceContext dc = GetDeviceContext();

		SamplerState result = {};
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

		DXCHECK(dc.device->CreateSamplerState(&sampDesc, &result.sampler));

		return result;
	}
}

#endif