#pragma once
#include "DX11Header.h"
#include "DX11Shader.h"
#include "DX11StaticMesh.h"
#include "DX11Texture.h"
#include "DX11Topology.h"


namespace cm
{
	namespace RenderCommand
	{
		void ClearRenderTarget(ID3D11RenderTargetView* target, const Vec4f& colour);
		void ClearDepthBuffer(ID3D11DepthStencilView* depth);

		void BindShader(const ShaderInstance& shader);
		void BindAndDrawMesh(const StaticMesh& mesh);

		void BindSampler(const SamplerInstance& sampler, int32 slot);
		void BindTexture(const TextureInstance& texture, int32 slot);

		void BindRenderTargets(ID3D11RenderTargetView* colour0, ID3D11DepthStencilView* depth);
		void BindRenderTargets(ID3D11RenderTargetView* colour0, ID3D11RenderTargetView* colour1, ID3D11RenderTargetView* colour2, ID3D11RenderTargetView* colour3, ID3D11DepthStencilView* depth);

		void SetTopology(const Topology& topology);
		void SetViewportState(real32 width, real32 height, real32 minDepth = 0.0f, real32 maxDepth = 1.0f, real32 topLeftX = 0.0f, real32 topLeftY = 0.0f);
		void SetRasterState(ID3D11RasterizerState* rasterState);
		void SetDepthState(ID3D11DepthStencilState* depthState);

		void BindShaderConstBuffer(ID3D11Buffer* buffer, ShaderStage stage, int32 slot);
		void UpdateConstBuffer(ID3D11Buffer* buffer, void* data);

		template<typename T>
		inline void BindShaderConstBuffer(const ShaderConstBuffer<T>& buffer, ShaderStage stage, int32 slot)
		{
			BindShaderConstBuffer(buffer.buffer, stage, slot);
		}

		template<typename T>
		inline void UpdateConstBuffer(ShaderConstBuffer<T>& buffer)
		{
			buffer.data.Prepare();
			UpdateConstBuffer(buffer.buffer, (void*)(&buffer.data));
		}
	}

}
