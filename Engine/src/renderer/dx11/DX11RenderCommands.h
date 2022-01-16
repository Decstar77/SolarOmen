#pragma once
#include "DX11Types.h"
#include "core/SolarMath.h"

namespace sol
{
	class RenderCommand
	{
	public:
		static void SetTopology(const Topology& topology);
		static void ClearRenderTarget(ID3D11RenderTargetView* target, const Vec4f& colour);
		static void ClearDepthBuffer(ID3D11DepthStencilView* depth);
		static void SetRenderTargets(ID3D11RenderTargetView* colour0, ID3D11DepthStencilView* depth);
		static void SetRenderTargets(ID3D11RenderTargetView* colour0, ID3D11RenderTargetView* colour1, ID3D11RenderTargetView* colour2, ID3D11RenderTargetView* colour3, ID3D11DepthStencilView* depth);
		static void SetViewportState(real32 width, real32 height, real32 minDepth = 0.0f, real32 maxDepth = 1.0f, real32 topLeftX = 0.0f, real32 topLeftY = 0.0f);
		static void SetRasterState(ID3D11RasterizerState* rasterState);
		static void SetDepthState(ID3D11DepthStencilState* depthState);

		static void SetProgram(const ProgramInstance& progam);
		static void SetStaticMesh(const StaticMesh& mesh);
		static void DrawStaticMesh(const StaticMesh& mesh);

		static void SetSampler(const SamplerState& sampler, int32 slot);
		static void SetTexture(const StaticTexture& texture, int32 slot);
		static void SetCubeMap(const CubeMapInstance& cubeMap, int32 slot);

		template<typename T>
		inline static void SetShaderConstBuffer(ShaderConstBuffer<T>* buffer, ShaderStage stage, int32 slot)
		{
			DeviceContext dc = GetDeviceContext();

			switch (stage)
			{
			case ShaderStage::VERTEX:
			{
				DXINFO(dc.context->VSSetConstantBuffers(slot, 1, &buffer->buffer));
			} break;
			case ShaderStage::PIXEL:
			{
				DXINFO(dc.context->PSSetConstantBuffers(slot, 1, &buffer->buffer));
			} break;
			case ShaderStage::COMPUTE:
			{
				DXINFO(dc.context->CSSetConstantBuffers(slot, 1, &buffer->buffer));
			} break;
			default:
			{
				Assert(0, "SetShaderConstBuffer invalid shader stage");
			} break;
			}
		}

		template<typename T>
		inline static void UploadShaderConstBuffer(ShaderConstBuffer<T>* buffer)
		{
			DeviceContext dc = GetDeviceContext();
			buffer->data.Prepare();
			DXINFO(dc.context->UpdateSubresource(buffer->buffer, 0, nullptr, (void*)&buffer->data, 0, 0));
		}

	};
}
