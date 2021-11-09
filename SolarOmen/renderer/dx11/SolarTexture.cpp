#include "../SolarRenderer.h"
namespace cm
{
	void SamplerInstance::Bind(RenderState* rs, int32 register_)
	{
		Assert(register_ >= 0, "Invalid sampler register");
		this->currentRegister = register_;
		DXINFO(rs->context->PSSetSamplers(register_, 1, &sampler));
		DXINFO(rs->context->CSSetSamplers(register_, 1, &sampler));
	}

	SamplerInstance SamplerInstance::Create(RenderState* rs, TextureFilterMode filter, TextureWrapMode wrap, int32 currentRegister)
	{
		SamplerInstance result = {};
		result.filter = filter;
		result.wrap = wrap;

		D3D11_SAMPLER_DESC sampDesc = {};
		sampDesc.Filter = GetTextureFilterModeToD3D(filter);

		sampDesc.AddressU = GetTextureWrapModeToD3D(wrap);
		sampDesc.AddressV = GetTextureWrapModeToD3D(wrap);
		sampDesc.AddressW = GetTextureWrapModeToD3D(wrap);

		DXCHECK(rs->device->CreateSamplerState(&sampDesc, &result.sampler));

		if (currentRegister >= 0)
		{
			result.Bind(rs, currentRegister);
		}

		return result;
	}

	SamplerInstance SamplerInstance::CreateShadowPFC(RenderState* rs, int32 currentRegister)
	{
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

		if (currentRegister >= 0)
		{
			result.Bind(rs, currentRegister);
		}

		return result;
	}

	void TextureInstance::Bind(RenderState* rs, ShaderStage shaderStage, int32 register_)
	{
		Assert(register_ >= 0, "Shader register invalid");

		this->currentRegister = register_;
		this->shaderStage = shaderStage;

		switch (shaderStage)
		{
		case ShaderStage::VERTEX: break;
		case ShaderStage::PIXEL:
		{
			Assert(shaderView, "Shader view is null");
			DXINFO(rs->context->PSSetShaderResources(register_, 1, &shaderView));
		}break;
		case ShaderStage::COMPUTE:
		{
			Assert(shaderView, "Shader view is null");
			DXINFO(rs->context->CSSetShaderResources(register_, 1, &shaderView));
		}break;
		}
	}

	void TextureInstance::Unbind(RenderState* rs)
	{
		ID3D11ShaderResourceView* nullTexture = nullptr;
		switch (shaderStage)
		{
		case ShaderStage::VERTEX: break;
		case ShaderStage::PIXEL:
		{
			DXINFO(rs->context->PSSetShaderResources(currentRegister, 1, &nullTexture));
		}break;
		case ShaderStage::COMPUTE:
		{
			DXINFO(rs->context->CSSetShaderResources(currentRegister, 1, &nullTexture));
		}break;
		}
	}

	void CubeMapInstance::Bind(RenderState* rs, ShaderStage shaderStage, int32 register_)
	{
		Assert(register_ >= 0, "Shader register invalid");

		switch (shaderStage)
		{
		case ShaderStage::VERTEX: break;
		case ShaderStage::PIXEL:
		{
			DXINFO(rs->context->PSSetShaderResources(register_, 1, &shaderView));
		}break;
		case ShaderStage::COMPUTE:
		{
			DXINFO(rs->context->CSSetShaderResources(register_, 1, &shaderView));
		}break;
		}
	}
}