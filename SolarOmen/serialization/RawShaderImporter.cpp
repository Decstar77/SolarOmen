#include "RawShaderImporter.h"

namespace cm
{
	ShaderAsset LoadShader(const CString& vertexPath, const CString& pixelPath)
	{
		PlatformFile vertexFile = Platform::LoadEntireFile(vertexPath, false);
		PlatformFile pixelFile = Platform::LoadEntireFile(pixelPath, false);

		ShaderAsset shaderAsset = {};

		uint32 vertexSize = SafeTruncateUInt64(vertexFile.sizeBytes);
		uint32 pixelSize = SafeTruncateUInt64(pixelFile.sizeBytes);

		shaderAsset.vertexLayout = VertexShaderLayoutType::Value::PNT;

		shaderAsset.vertexData = ManagedArray<char>(vertexSize, MemoryType::PERMANENT);
		shaderAsset.vertexData.count = vertexSize;
		memcpy(shaderAsset.vertexData.data, vertexFile.data, vertexSize);

		shaderAsset.pixelData = ManagedArray<char>(pixelSize, MemoryType::PERMANENT);
		shaderAsset.pixelData.count = pixelSize;
		memcpy(shaderAsset.pixelData.data, pixelFile.data, pixelSize);

		return shaderAsset;
	}
}
