#include "DX11Types.h"
#include "core/SolarLogging.h"


namespace sol
{
	void ProgramInstance::Release(ProgramInstance* program)
	{
		DeviceContext dc = GetDeviceContext();
		DXRELEASE(program->vs);
		DXRELEASE(program->ps);
		DXRELEASE(program->cs);
		DXRELEASE(program->layout);

		GameMemory::ZeroStruct(program);
	}

	ProgramInstance ProgramInstance::CreateGraphics(const ProgramResource& programResource)
	{
		DeviceContext dc = GetDeviceContext();

		ProgramInstance program = {};
		program.id = programResource.id;

		switch (programResource.vertexLayout.Get())
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

			DXCHECK(dc.device->CreateInputLayout(layouts, ArrayCount(layouts), programResource.vertexData.data,
				programResource.vertexData.GetCount(), &program.layout));

			DXCHECK(dc.device->CreateVertexShader(programResource.vertexData.data,
				programResource.vertexData.GetCount(), nullptr, &program.vs));

			DXCHECK(dc.device->CreatePixelShader(programResource.pixelData.data,
				programResource.pixelData.GetCount(), nullptr, &program.ps));

			SOLTRACE(String("Created program: ").Add(programResource.name).GetCStr());
			return program;
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

			DXCHECK(dc.device->CreateInputLayout(layouts, ArrayCount(layouts), programResource.vertexData.data,
				programResource.vertexData.GetCount(), &program.layout));

			DXCHECK(dc.device->CreateVertexShader(programResource.vertexData.data,
				programResource.vertexData.GetCount(), nullptr, &program.vs));

			DXCHECK(dc.device->CreatePixelShader(programResource.pixelData.data,
				programResource.pixelData.GetCount(), nullptr, &program.ps));

			SOLTRACE(String("Created program: ").Add(programResource.name).GetCStr());
			return program;
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

			DXCHECK(dc.device->CreateInputLayout(layouts, ArrayCount(layouts), programResource.vertexData.data,
				programResource.vertexData.GetCount(), &program.layout));

			DXCHECK(dc.device->CreateVertexShader(programResource.vertexData.data,
				programResource.vertexData.GetCount(), nullptr, &program.vs));

			DXCHECK(dc.device->CreatePixelShader(programResource.pixelData.data,
				programResource.pixelData.GetCount(), nullptr, &program.ps));

			SOLTRACE(String("Created program: ").Add(programResource.name).GetCStr());
			return program;
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

			ProgramInstance program = {};
			program.id = programResource.id;

			DXCHECK(dc.device->CreateInputLayout(layouts, ArrayCount(layouts), programResource.vertexData.data,
				programResource.vertexData.GetCount(), &program.layout));

			DXCHECK(dc.device->CreateVertexShader(programResource.vertexData.data,
				programResource.vertexData.GetCount(), nullptr, &program.vs));

			DXCHECK(dc.device->CreatePixelShader(programResource.pixelData.data,
				programResource.pixelData.GetCount(), nullptr, &program.ps));

			SOLTRACE(String("Created program: ").Add(programResource.name).GetCStr());
			return program;

		}break;
		}

		SOLFATAL("Shader program -> CreateGraphics unkown format");
		Assert(0, "ShaderInstance::CreateGraphics unkown format");

		return {};
	}
}