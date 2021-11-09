#include "SolarShader.h"
#include "../SolarRenderer.h"
namespace cm
{
	void ShaderInstance::Bind()
	{
		RenderState* rs = RenderState::GlobalRenderState;

		if (vs)
		{
			DXINFO(rs->context->VSSetShader(vs, nullptr, 0));
			DXINFO(rs->context->IASetInputLayout(layout));
		}
		if (ps)
		{
			DXINFO(rs->context->PSSetShader(ps, nullptr, 0));
		}
		if (cs)
		{
			DXINFO(rs->context->CSSetShader(cs, nullptr, 0));
		}
	}

	ShaderInstance ShaderInstance::CreateGraphics(ShaderData* shaderData, VertexShaderLayout layout)
	{
		RenderState* rs = RenderState::GlobalRenderState;

		switch (layout)
		{
		case VertexShaderLayout::P:
		{
			D3D11_INPUT_ELEMENT_DESC pos_desc = {};
			pos_desc.SemanticName = "Position";
			pos_desc.SemanticIndex = 0;
			pos_desc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
			pos_desc.InputSlot = 0;
			pos_desc.AlignedByteOffset = 0;
			pos_desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			pos_desc.InstanceDataStepRate = 0;

			D3D11_INPUT_ELEMENT_DESC layouts[] = { pos_desc };

			ShaderInstance shader = {};
			shader.id = shaderData->id;
			DXCHECK(rs->device->CreateInputLayout(layouts, 1, shaderData->vertexData,
				shaderData->vertexSizeBytes, &shader.layout));

			DXCHECK(rs->device->CreateVertexShader(shaderData->vertexData,
				shaderData->vertexSizeBytes, nullptr, &shader.vs));

			DXCHECK(rs->device->CreatePixelShader(shaderData->pixelData,
				shaderData->pixelSizeBytes, nullptr, &shader.ps));

			int32 index = (int32)shaderData->id;
			Assert(index != 0, "Error");

			rs->shaders[index] = shader;

			return shader;
		}break;

		case VertexShaderLayout::PNT:
		{
			D3D11_INPUT_ELEMENT_DESC pos_desc = {};
			pos_desc.SemanticName = "Position";
			pos_desc.SemanticIndex = 0;
			pos_desc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
			pos_desc.InputSlot = 0;
			pos_desc.AlignedByteOffset = 0;
			pos_desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			pos_desc.InstanceDataStepRate = 0;

			D3D11_INPUT_ELEMENT_DESC nrm_desc = {};
			nrm_desc.SemanticName = "Normal";
			nrm_desc.SemanticIndex = 0;
			nrm_desc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
			nrm_desc.InputSlot = 0;
			nrm_desc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			nrm_desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			nrm_desc.InstanceDataStepRate = 0;

			D3D11_INPUT_ELEMENT_DESC txc_desc = {};
			txc_desc.SemanticName = "TexCord";
			txc_desc.SemanticIndex = 0;
			txc_desc.Format = DXGI_FORMAT_R32G32_FLOAT;
			txc_desc.InputSlot = 0;
			txc_desc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			txc_desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			txc_desc.InstanceDataStepRate = 0;

			D3D11_INPUT_ELEMENT_DESC layouts[] = { pos_desc, nrm_desc, txc_desc };

			ShaderInstance shader = {};
			shader.id = shaderData->id;

			DXCHECK(rs->device->CreateInputLayout(layouts, 3, shaderData->vertexData,
				shaderData->vertexSizeBytes, &shader.layout));

			DXCHECK(rs->device->CreateVertexShader(shaderData->vertexData,
				shaderData->vertexSizeBytes, nullptr, &shader.vs));

			DXCHECK(rs->device->CreatePixelShader(shaderData->pixelData,
				shaderData->pixelSizeBytes, nullptr, &shader.ps));

			int32 index = (int32)shaderData->id;
			Assert(index != 0, "Error");

			rs->shaders[index] = shader;

			return shader;
		}break;
		}

		return {};
	}

	ShaderInstance ShaderInstance::CreateCompute(ShaderData* shaderData)
	{
		RenderState* rs = RenderState::GlobalRenderState;

		ShaderInstance result = {};
		result.id = shaderData->id;

		DXCHECK(rs->device->CreateComputeShader(shaderData->computeData, shaderData->computeSizeBytes,
			nullptr, &result.cs));

		int32 index = (int32)shaderData->id;
		Assert(index != 0, "Error");
		rs->shaders[index] = result;

		return result;
	}

	void ShaderConstBuffer::CommitChanges()
	{
		RenderState* rs = RenderState::GlobalRenderState;

		DXINFO(rs->context->UpdateSubresource(buffer, 0, nullptr, (void*)stagingBuffer, 0, 0));
		ZeroMemory(stagingBuffer, sizeBytes);
		copy_ptr = 0;
	}

	void ShaderConstBuffer::BindShaderBuffer(ShaderStage stage, int32 register_)
	{
		RenderState* rs = RenderState::GlobalRenderState;

		switch (stage)
		{
		case ShaderStage::VERTEX:
		{
			DXINFO(rs->context->VSSetConstantBuffers(register_, 1, &buffer));
		}break;
		case ShaderStage::PIXEL:
		{
			DXINFO(rs->context->PSSetConstantBuffers(register_, 1, &buffer));
		}break;
		case ShaderStage::COMPUTE:
		{
			DXINFO(rs->context->CSSetConstantBuffers(register_, 1, &buffer));
		}break;
		default:
		{
			Assert(0, "INVALID CODE PATH");
		}break;
		}
	}

	void ShaderConstBuffer::CopyVec3fIntoShaderBuffer(const Vec3f& a)
	{
		Assert(copy_ptr + 4 <= sizeBytes / 4, "DXConstBuffer::CopyInVec3f buffer overrun");

		real32* ptr = (real32*)stagingBuffer;

		ptr[copy_ptr] = a.x;
		copy_ptr++;
		ptr[copy_ptr] = a.y;
		copy_ptr++;
		ptr[copy_ptr] = a.z;
		copy_ptr++;
		ptr[copy_ptr] = 0.0f;
		copy_ptr++;
	}

	void ShaderConstBuffer::CopyVec4iIntoShaderBuffer(const Vec4i& a)
	{
		Assert(copy_ptr + 4 <= sizeBytes / 4, "DXConstBuffer::CopyInVec4i buffer overrun");

		int32* ptr = (int32*)stagingBuffer;

		ptr[copy_ptr] = a.x;
		copy_ptr++;
		ptr[copy_ptr] = a.y;
		copy_ptr++;
		ptr[copy_ptr] = a.z;
		copy_ptr++;
		ptr[copy_ptr] = 0;
		copy_ptr++;
	}

	void ShaderConstBuffer::CopyVec4fIntoShaderBuffer(const Vec4f& a)
	{
		Assert(copy_ptr + 4 <= sizeBytes / 4, "DXConstBuffer::CopyInVec3f buffer overrun");

		real32* ptr = (real32*)stagingBuffer;

		ptr[copy_ptr] = a.x;
		copy_ptr++;
		ptr[copy_ptr] = a.y;
		copy_ptr++;
		ptr[copy_ptr] = a.z;
		copy_ptr++;
		ptr[copy_ptr] = a.w;
		copy_ptr++;
	}

	void ShaderConstBuffer::CopyMat4fIntoShaderBuffer(const Mat4f& a, bool32 transpose)
	{
		Assert(copy_ptr + 16 <= sizeBytes / 4, "DXConstBuffer::CopyInMat4f buffer overrun");

		real32* ptr = (real32*)stagingBuffer;

		// @TODO: There is probably a faster way, without the call to transopse
		Mat4f aT = Transpose(a);
		for (int32 i = 0; i < 16; i++)
		{
			ptr[copy_ptr] = aT.ptr[i];
			copy_ptr++;
		}
	}
}
