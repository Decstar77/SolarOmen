#include "DX12Types.h"
#include "core/SolarLogging.h"
#include "d3dx12.h"

#if SOL_DEBUG_RENDERING
#include <D3Dcompiler.h>
#endif
namespace sol
{
#if SOL_DEBUG_RENDERING

	inline std::wstring AnsiToWString(const std::string& str)
	{
		WCHAR buffer[512];
		MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
		return std::wstring(buffer);
	}

	static ID3DBlob* CompileShader(const String& path, const char* entry, const char* target)
	{
		ID3DBlob* shader = nullptr;
		ID3DBlob* errorBuff = nullptr;

		HRESULT hr = D3DCompileFromFile(
			AnsiToWString(path.GetCStr()).c_str(),
			nullptr, nullptr,
			entry, target,
			D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
			0, &shader, &errorBuff);

		if (FAILED(hr))
		{
			OutputDebugStringA((char*)errorBuff->GetBufferPointer());
			SOLERROR((char*)errorBuff->GetBufferPointer());
			return nullptr;
		}

		return shader;
	}

	StaticProgram StaticProgram::DebugCreateCompile(const String& programPath, VertexLayoutType layout)
	{
		ID3DBlob* vertexShader = CompileShader(programPath, "VSmain", "vs_5_0");
		ID3DBlob* pixelShader = CompileShader(programPath, "PSmain", "ps_5_0");

		if (!(vertexShader && pixelShader))
		{
			SOLERROR("Could not compile program");
			return {};
		}

		StaticProgram program = {};
		program.vertexShaderByteCode.BytecodeLength = vertexShader->GetBufferSize();
		program.vertexShaderByteCode.pShaderBytecode = vertexShader->GetBufferPointer();
		program.pixelShaderByteCode.BytecodeLength = pixelShader->GetBufferSize();
		program.pixelShaderByteCode.pShaderBytecode = pixelShader->GetBufferPointer();
		program.layout = layout;

		return program;
	}

#endif
}