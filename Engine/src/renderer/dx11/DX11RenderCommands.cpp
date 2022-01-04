#include "DX11RenderCommands.h"

namespace sol
{
	void RenderCommand::ClearRenderTarget(ID3D11RenderTargetView* target, const Vec4f& colour)
	{
		DeviceContext dc = GetDeviceContext();
		DXINFO(dc.context->ClearRenderTargetView(target, colour.ptr));
	}

	void RenderCommand::ClearDepthBuffer(ID3D11DepthStencilView* depth)
	{
		DeviceContext dc = GetDeviceContext();
		DXINFO(dc.context->ClearDepthStencilView(depth, D3D11_CLEAR_DEPTH, 1.0f, 0))
	}

	void RenderCommand::SetRenderTargets(ID3D11RenderTargetView* colour0, ID3D11DepthStencilView* depth)
	{
		DeviceContext dc = GetDeviceContext();
		ID3D11RenderTargetView* views[] = { colour0 };
		DXINFO(dc.context->OMSetRenderTargets(1, views, depth));
	}

	void  RenderCommand::SetRenderTargets(
		ID3D11RenderTargetView* colour0,
		ID3D11RenderTargetView* colour1,
		ID3D11RenderTargetView* colour2,
		ID3D11RenderTargetView* colour3,
		ID3D11DepthStencilView* depth)
	{
		DeviceContext dc = GetDeviceContext();
		ID3D11RenderTargetView* views[] = { colour0, colour1, colour2, colour3 };
		DXINFO(dc.context->OMSetRenderTargets(4, views, depth));
	}

	void RenderCommand::SetTopology(const Topology& topology)
	{
		DeviceContext dc = GetDeviceContext();
		DXINFO(dc.context->IASetPrimitiveTopology(topology.GetDXFormat()));
	}

	void RenderCommand::SetViewportState(real32 width, real32 height, real32 minDepth, real32 maxDepth, real32 topLeftX, real32 topLeftY)
	{
		D3D11_VIEWPORT viewport = {};

		viewport.Width = width;
		viewport.Height = height;
		viewport.MinDepth = minDepth;
		viewport.MaxDepth = maxDepth;
		viewport.TopLeftX = topLeftX;
		viewport.TopLeftY = topLeftY;

		DeviceContext dc = GetDeviceContext();
		DXINFO(dc.context->RSSetViewports(1, &viewport));
	}

	void RenderCommand::SetRasterState(ID3D11RasterizerState* rasterState)
	{
		DeviceContext dc = GetDeviceContext();
		DXINFO(dc.context->RSSetState(rasterState));
	}

	void RenderCommand::SetDepthState(ID3D11DepthStencilState* depthState)
	{
		DeviceContext dc = GetDeviceContext();
		DXINFO(dc.context->OMSetDepthStencilState(depthState, 1));
	}

	//void BindShader(const ShaderInstance& shader)
	//{
	//	GetRenderState();
	//	if (shader.vs)
	//	{
	//		DXINFO(rs->context->VSSetShader(shader.vs, nullptr, 0));
	//		DXINFO(rs->context->IASetInputLayout(shader.layout));
	//	}
	//	if (shader.ps)
	//	{
	//		DXINFO(rs->context->PSSetShader(shader.ps, nullptr, 0));
	//	}
	//	if (shader.cs)
	//	{
	//		DXINFO(rs->context->CSSetShader(shader.cs, nullptr, 0));
	//	}
	//}

	//void BindAndDrawMesh(const StaticMesh& mesh)
	//{
	//	GetRenderState();
	//	uint32 offset = 0;
	//	DXINFO(rs->context->IASetVertexBuffers(0, 1, &mesh.vertexBuffer, &mesh.strideBytes, &offset));
	//	DXINFO(rs->context->IASetIndexBuffer(mesh.indexBuffer, DXGI_FORMAT_R32_UINT, 0));
	//	DXINFO(rs->context->DrawIndexed(mesh.indexCount, 0, 0));
	//}

	//void BindSampler(const SamplerInstance& sampler, int32 slot)
	//{
	//	Assert(slot >= 0, "Shader register invalid");
	//	GetRenderState();

	//	Assert(slot >= 0, "Invalid sampler register");
	//	DXINFO(rs->context->PSSetSamplers(slot, 1, &sampler.sampler));
	//	DXINFO(rs->context->CSSetSamplers(slot, 1, &sampler.sampler));
	//}

	//void BindTexture(const TextureInstance& texture, int32 slot)
	//{
	//	Assert(slot >= 0, "Shader register invalid");
	//	GetRenderState();

	//	Assert(slot >= 0, "Shader register invalid");
	//	DXINFO(rs->context->PSSetShaderResources(slot, 1, &texture.shaderView));
	//	DXINFO(rs->context->CSSetShaderResources(slot, 1, &texture.shaderView));
	//}

	//void BindCubeMap(const CubeMapInstance& cubeMap, int32 slot)
	//{
	//	Assert(slot >= 0, "Shader register invalid");
	//	GetRenderState();

	//	DXINFO(rs->context->PSSetShaderResources(slot, 1, &cubeMap.shaderView));
	//	DXINFO(rs->context->CSSetShaderResources(slot, 1, &cubeMap.shaderView));
	//}

	//void BindShaderConstBuffer(ID3D11Buffer* buffer, ShaderStage stage, int32 slot)
	//{
	//	GetRenderState();
	//	switch (stage)
	//	{
	//	case ShaderStage::VERTEX:
	//	{
	//		DXINFO(rs->context->VSSetConstantBuffers(slot, 1, &buffer));
	//	}break;
	//	case ShaderStage::PIXEL:
	//	{
	//		DXINFO(rs->context->PSSetConstantBuffers(slot, 1, &buffer));
	//	}break;
	//	case ShaderStage::COMPUTE:
	//	{
	//		DXINFO(rs->context->CSSetConstantBuffers(slot, 1, &buffer));
	//	}break;
	//	default:
	//	{
	//		Assert(0, "INVALID CODE PATH");
	//	}break;
	//	}
	//}

	//void UpdateConstBuffer(ID3D11Buffer* buffer, void* data)
	//{
	//	GetRenderState();
	//	DXINFO(rs->context->UpdateSubresource(buffer, 0, nullptr, data, 0, 0));
	//}

}