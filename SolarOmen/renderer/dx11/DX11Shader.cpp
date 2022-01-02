#include "DX11Shader.h"
#include "DX11Renderer.h"

namespace cm
{
	ShaderInstance ShaderInstance::CreateGraphics(const ShaderAsset& shaderAsset)
	{
		GetRenderState();

		switch (shaderAsset.vertexLayout.Get())
		{
		case VertexLayoutType::Value::P:
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
			shader.id = shaderAsset.id;
			DXCHECK(rs->device->CreateInputLayout(layouts, ArrayCount(layouts), shaderAsset.vertexData.data,
				shaderAsset.vertexData.GetCount(), &shader.layout));

			DXCHECK(rs->device->CreateVertexShader(shaderAsset.vertexData.data,
				shaderAsset.vertexData.GetCount(), nullptr, &shader.vs));

			DXCHECK(rs->device->CreatePixelShader(shaderAsset.pixelData.data,
				shaderAsset.pixelData.GetCount(), nullptr, &shader.ps));

			return shader;
		}break;

		case VertexLayoutType::Value::PNT:
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
			shader.id = shaderAsset.id;

			DXCHECK(rs->device->CreateInputLayout(layouts, ArrayCount(layouts), shaderAsset.vertexData.data,
				shaderAsset.vertexData.GetCount(), &shader.layout));

			DXCHECK(rs->device->CreateVertexShader(shaderAsset.vertexData.data,
				shaderAsset.vertexData.GetCount(), nullptr, &shader.vs));

			DXCHECK(rs->device->CreatePixelShader(shaderAsset.pixelData.data,
				shaderAsset.pixelData.GetCount(), nullptr, &shader.ps));

			return shader;
		}break;
		case VertexLayoutType::Value::PNTC:
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

			D3D11_INPUT_ELEMENT_DESC col_desc = {};
			col_desc.SemanticName = "Colour";
			col_desc.SemanticIndex = 0;
			col_desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			col_desc.InputSlot = 0;
			col_desc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			col_desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			col_desc.InstanceDataStepRate = 0;

			D3D11_INPUT_ELEMENT_DESC layouts[] = { pos_desc, nrm_desc, txc_desc, col_desc };

			ShaderInstance shader = {};
			shader.id = shaderAsset.id;

			DXCHECK(rs->device->CreateInputLayout(layouts, ArrayCount(layouts), shaderAsset.vertexData.data,
				shaderAsset.vertexData.GetCount(), &shader.layout));

			DXCHECK(rs->device->CreateVertexShader(shaderAsset.vertexData.data,
				shaderAsset.vertexData.GetCount(), nullptr, &shader.vs));

			DXCHECK(rs->device->CreatePixelShader(shaderAsset.pixelData.data,
				shaderAsset.pixelData.GetCount(), nullptr, &shader.ps));

			return shader;
		}break;
		case VertexLayoutType::Value::TEXT:
		{
			D3D11_INPUT_ELEMENT_DESC desc = {};
			desc.SemanticName = "POSITION";
			desc.SemanticIndex = 0;
			desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			desc.InputSlot = 0;
			desc.AlignedByteOffset = 0;
			desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			desc.InstanceDataStepRate = 0;
			D3D11_INPUT_ELEMENT_DESC layouts[] = { desc };

			ShaderInstance shader = {};
			shader.id = shaderAsset.id;

			DXCHECK(rs->device->CreateInputLayout(layouts, ArrayCount(layouts), shaderAsset.vertexData.data,
				shaderAsset.vertexData.GetCount(), &shader.layout));

			DXCHECK(rs->device->CreateVertexShader(shaderAsset.vertexData.data,
				shaderAsset.vertexData.GetCount(), nullptr, &shader.vs));

			DXCHECK(rs->device->CreatePixelShader(shaderAsset.pixelData.data,
				shaderAsset.pixelData.GetCount(), nullptr, &shader.ps));

			return shader;

		}break;
		}

		Assert(0, "ShaderInstance::CreateGraphics unkown format");

		return {};
	}

	/*ShaderInstance ShaderInstance::CreateCompute(ShaderData* shaderData)
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
	}*/


	ID3D11Buffer* CreateShaderConstBuffer(int32 size)
	{
		GetRenderState();

		D3D11_BUFFER_DESC desc = {};
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.Usage = D3D11_USAGE_DEFAULT; //D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		desc.ByteWidth = size;
		desc.StructureByteStride = 0;

		ID3D11Buffer* buffer = nullptr;
		DXCHECK(rs->device->CreateBuffer(&desc, NULL, &buffer));

		return buffer;
	}




	static ShaderConstBuffer2 CreateShaderBuffer(RenderState* rs, int32 sizeBytes)
	{
		D3D11_BUFFER_DESC desc = {};
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.Usage = D3D11_USAGE_DEFAULT; //D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		desc.ByteWidth = sizeBytes;
		desc.StructureByteStride = 0;

		ShaderConstBuffer2 result = {};

		DXCHECK(rs->device->CreateBuffer(&desc, NULL, &result.buffer));
		result.sizeBytes = sizeBytes;
		result.copy_ptr = 0;
		Assert(ArrayCount(result.stagingBuffer) > sizeBytes, "Shader buffer staging buffer is not big enough");

		return result;
	}


	void ShaderConstBuffer2::CommitChanges()
	{
		GetRenderState();

		DXINFO(rs->context->UpdateSubresource(buffer, 0, nullptr, (void*)stagingBuffer, 0, 0));
		ZeroMemory(stagingBuffer, sizeBytes);
		copy_ptr = 0;
	}

	void ShaderConstBuffer2::BindShaderBuffer(ShaderStage stage, int32 register_)
	{
		GetRenderState();

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

	void ShaderConstBuffer2::CopyVec3fIntoShaderBuffer(const Vec3f& a)
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

	void ShaderConstBuffer2::CopyVec4iIntoShaderBuffer(const Vec4i& a)
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

	void ShaderConstBuffer2::CopyVec4fIntoShaderBuffer(const Vec4f& a)
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

	void ShaderConstBuffer2::CopyMat4fIntoShaderBuffer(const Mat4f& a, bool32 transpose)
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
