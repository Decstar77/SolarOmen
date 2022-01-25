#include "DX11Techniques.h"
#include "DX11RenderCommands.h"

namespace sol
{
	CubeMapInstance RenderTechnique::ConvertEqiTextureToCubeMap(RenderState* rs, uint32 resolution, const StaticTexture& eqi)
	{
		CubeMapInstance result = CubeMapInstance::Create(resolution);

		RenderCommand::SetViewportState((real32)resolution, (real32)resolution);
		RenderCommand::SetProgram(rs->eqiToCubeProgram);

		RenderCommand::SetTexture(eqi, 0);
		RenderCommand::SetRasterState(rs->rasterNoFaceCullState);
		RenderCommand::SetDepthState(rs->depthOffState);
		RenderCommand::SetTopology(Topology::Value::TRIANGLE_LIST);

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
			rs->viewConstBuffer.data.view = views[i];
			RenderCommand::UploadShaderConstBuffer(&rs->viewConstBuffer);

			RenderCommand::ClearRenderTarget(result.renderFaces[i], Vec4f(0, 1, 0, 1));
			RenderCommand::SetRenderTargets(result.renderFaces[i], nullptr);

			RenderCommand::DrawStaticMesh(rs->cube);
		}

		RenderCommand::SetRenderTargets(nullptr, nullptr);

		return result;
	}

	//CubeMapInstance RenderTechnique::ConvoluteCubeMap(uint32 resolution, const CubeMapInstance& cube)
	//{
	//	GetRenderState();
	//	CubeMapInstance result = CubeMapInstance::Create(resolution);

	//	RenderCommand::SetViewportState((real32)resolution, (real32)resolution);
	//	RenderCommand::BindShader(rs->irradianceConvolutionShader);

	//	RenderCommand::BindCubeMap(cube, 10);
	//	RenderCommand::SetRasterState(rs->rasterNoFaceCullState);
	//	RenderCommand::SetDepthState(rs->depthOffState);
	//	RenderCommand::SetTopology(Topology::Value::TRIANGLE_LIST);

	//	Mat4f views[6] =
	//	{
	//		Inverse(LookAtLH(Vec3f(0), Vec3f(1, 0, 0), Vec3f(0, 1, 0))),
	//		Inverse(LookAtLH(Vec3f(0), Vec3f(-1, 0, 0), Vec3f(0, 1, 0))),

	//		Inverse(LookAtLH(Vec3f(0), Vec3f(0, 1, 0), Vec3f(0, 0, -1))),
	//		Inverse(LookAtLH(Vec3f(0), Vec3f(0, -1, 0), Vec3f(0, 0, 1))),

	//		Inverse(LookAtLH(Vec3f(0), Vec3f(0, 0, 1), Vec3f(0, 1, 0))),
	//		Inverse(LookAtLH(Vec3f(0), Vec3f(0, 0, -1), Vec3f(0, 1, 0)))
	//	};

	//	Mat4f proj = PerspectiveLH(DegToRad(90.0f), 1.0f, 0.1f, 10.0f);
	//	for (int32 i = 0; i < 6; i++)
	//	{
	//		rs->viewConstBuffer.data.persp = proj;
	//		rs->viewConstBuffer.data.persp = views[i];
	//		RenderCommand::UpdateConstBuffer(rs->viewConstBuffer);

	//		RenderCommand::ClearRenderTarget(result.renderFaces[i], Vec4f(0, 1, 0, 1));
	//		RenderCommand::BindRenderTargets(result.renderFaces[i], nullptr);

	//		RenderCommand::BindAndDrawMesh(rs->cube);
	//	}

	//	RenderCommand::BindRenderTargets(nullptr, nullptr);

	//	return result;
	//}

}