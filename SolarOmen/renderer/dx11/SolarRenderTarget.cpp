#include "../SolarRenderer.h"
namespace cm
{
	void RenderTarget::Clear(RenderState* rs, const Vec4f& colour)
	{
		for (int32 i = 0; i < ArrayCount(colourTargets); i++)
		{
			if (colourTargets[i].IsValid())
			{
				DXINFO(rs->context->ClearRenderTargetView(colourTargets[i].renderView, colour.ptr));
			}
		}

		if (depthTarget.IsValid())
		{
			DXINFO(rs->context->ClearDepthStencilView(depthTarget.depthView, D3D11_CLEAR_DEPTH, 1.0f, 0))
		}
	}

	void RenderTarget::Bind(RenderState* rs)
	{
		ID3D11RenderTargetView* views[4] = { nullptr, nullptr, nullptr, nullptr };

		for (int32 i = 0; i < ArrayCount(colourTargets); i++)
		{
			if (colourTargets[i].IsValid())
			{
				views[i] = colourTargets[i].renderView;
			}
		}

		if (depthTarget.IsValid())
		{
			DXINFO(rs->context->OMSetRenderTargets(4, views, depthTarget.depthView));
		}
		else
		{
			DXINFO(rs->context->OMSetRenderTargets(4, views, NULL));
		}

	}

	void RenderTarget::Unbind(RenderState* rs)
	{
		ID3D11RenderTargetView* nullTarget[4] = { nullptr, nullptr, nullptr, nullptr };
		DXINFO(rs->context->OMSetRenderTargets(4, nullTarget, nullptr));
	}
}