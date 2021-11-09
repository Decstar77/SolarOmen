#include "../SolarRenderer.h"
namespace cm
{
	void MeshInstance::Bind(RenderState* rs)
	{
		uint32 offset = 0;
		DXINFO(rs->context->IASetVertexBuffers(0, 1, &vertexBuffer, &strideBytes, &offset));
		DXINFO(rs->context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0));
	}

	void MeshInstance::DrawIndexed(RenderState* rs)
	{
		DXINFO(rs->context->DrawIndexed(indexCount, 0, 0));
	}
}