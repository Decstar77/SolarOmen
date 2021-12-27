#pragma once
#include "core/SolarCore.h"
#include "DX11Header.h"

namespace cm
{
	struct ShaderInstance
	{
		AssetId id;
		ID3D11VertexShader* vs;
		ID3D11PixelShader* ps;
		ID3D11ComputeShader* cs;
		ID3D11InputLayout* layout;

		static ShaderInstance CreateGraphics(const ShaderAsset& shaderAsset);
	};

	ID3D11Buffer* CreateShaderConstBuffer(int32 size);

	struct ShaderConstBuffer2
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


	template<typename T>
	struct ShaderConstBuffer
	{
		ID3D11Buffer* buffer;
		T data;

		static ShaderConstBuffer<T> Create();
	};


#define Float4Align __declspec(align(16))

	struct ShaderConstBufferModel
	{
		Float4Align Mat4f mvp;
		Float4Align Mat4f model;
		Float4Align Mat4f invM;

		inline void Prepare()
		{
			mvp = Transpose(mvp);
			model = Transpose(model);
			invM = Transpose(invM);
		}
	};

	struct ShaderConstBufferView
	{
		Float4Align Mat4f persp;
		Float4Align Mat4f view;
		Float4Align Mat4f screeenProjection;

		inline void Prepare()
		{
			persp = Transpose(persp);
			view = Transpose(view);
			screeenProjection = Transpose(screeenProjection);
		}
	};

	struct ShaderConstBufferLightingInfo
	{
		Float4Align Vec4f viewPos;
		Float4Align struct {
			int32 dirLightCount;
			int32 spotLightCount;
			int32 pointLightCount;
			int32 pad;
		};

		inline void Prepare()
		{

		}
	};

	struct ShaderConstBufferUIData
	{
		Float4Align Vec4f colour;
		Float4Align Vec4f sizePos;
		Float4Align Vec4i uiUses;

		inline void Prepare()
		{

		}
	};


	template<typename T>
	inline ShaderConstBuffer<T> ShaderConstBuffer<T>::Create()
	{
		ShaderConstBuffer result = {};
		result.buffer = CreateShaderConstBuffer(sizeof(T));

		return result;
	}

}
