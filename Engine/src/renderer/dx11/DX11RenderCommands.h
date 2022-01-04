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

		//void BindShader(const ShaderInstance& shader);
		//void BindAndDrawMesh(const StaticMesh& mesh);

		//void BindSampler(const SamplerInstance& sampler, int32 slot);
		//void BindTexture(const TextureInstance& texture, int32 slot);
		//void BindCubeMap(const CubeMapInstance& cubeMap, int32 slot);

		//void BindShaderConstBuffer(ID3D11Buffer* buffer, ShaderStage stage, int32 slot);
		//void UpdateConstBuffer(ID3D11Buffer* buffer, void* data);
	};
}
