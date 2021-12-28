#include "DX11Techniques.h"
#include "DX11Renderer.h"

namespace cm
{
	CubeMapInstance ConvertEqiTextureToCubeMap(uint32 resolution, const TextureInstance& eqi)
	{
		GetRenderState();

		CubeMapInstance cubeMap = CubeMapInstance::Create(resolution);

		RenderCommand::SetViewportState((real32)resolution, (real32)resolution);
		RenderCommand::BindShader(rs->eqiToCubeShader);

		RenderCommand::BindTexture(eqi, 0);
		RenderCommand::SetRasterState(rs->rasterNoFaceCullState);
		RenderCommand::SetDepthState(rs->depthOffState);

		Mat4f views[6] =
		{
			Inverse(LookAtLH(Vec3f(0), Vec3f(1, 0, 0), Vec3f(0, 1, 0))),
			Inverse(LookAtLH(Vec3f(0), Vec3f(-1, 0, 0), Vec3f(0, 1, 0))),

			Inverse(LookAtLH(Vec3f(0), Vec3f(0, 1, 0), Vec3f(0, 0, -1))),
			Inverse(LookAtLH(Vec3f(0), Vec3f(0, -1, 0), Vec3f(0, 0, 1))),

			Inverse(LookAtLH(Vec3f(0), Vec3f(0, 0, 1), Vec3f(0, 1, 0))),
			Inverse(LookAtLH(Vec3f(0), Vec3f(0, 0, -1), Vec3f(0, 1, 0)))
		};

		Mat4f proj = PerspectiveLH(DegToRad(90.0f), 1.0f, 0.1f, 10.0f);
		for (int32 i = 0; i < 6; i++)
		{
			rs->viewConstBuffer.data.persp = proj;
			rs->viewConstBuffer.data.persp = views[i];
			RenderCommand::UpdateConstBuffer(rs->viewConstBuffer);

			RenderCommand::ClearRenderTarget(cubeMap.renderFaces[i], Vec4f(0, 1, 0, 1));
			RenderCommand::BindRenderTargets(cubeMap.renderFaces[i], nullptr);

			RenderCommand::BindAndDrawMesh(rs->cube);
		}

		return cubeMap;
	}
}
