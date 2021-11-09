#pragma once
#include "core/SolarCore.h"

#include "../../SolarAssets.h"
#include "D3D11Header.h"

namespace cm
{
	struct ShaderInstance
	{
		ShaderId id;
		ID3D11VertexShader* vs;
		ID3D11PixelShader* ps;
		ID3D11ComputeShader* cs;
		ID3D11InputLayout* layout;

		static ShaderInstance CreateGraphics(ShaderData* shaderData, VertexShaderLayout layout);
		static ShaderInstance CreateCompute(ShaderData* shaderData);

		void Bind();
	};

	struct ShaderConstBuffer
	{
		ID3D11Buffer* buffer;
		int32 sizeBytes;
		int32 copy_ptr;
		uint8 stagingBuffer[2048];

		void CommitChanges();
		void BindShaderBuffer(ShaderStage stage, int32 register_);

		void CopyVec3fIntoShaderBuffer(const Vec3f& a);
		void CopyVec4iIntoShaderBuffer(const Vec4i& a);
		void CopyVec4fIntoShaderBuffer(const Vec4f& a);
		void CopyMat4fIntoShaderBuffer(const Mat4f& a, bool32 transpose);
	};
}
