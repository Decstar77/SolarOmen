#pragma once
#include "core/SolarCore.h"
#include "SimpleColliders.h"
#include "serialization/assetId/ShaderId.h"
#include "serialization/assetId/ModelId.h"
#include "serialization/assetId/TextureId.h"
#include "serialization/assetId/WorldId.h"

namespace cm
{
	enum class ShaderStage
	{
		VERTEX,
		PIXEL,
		COMPUTE,
	};

	enum class TextureFormat
	{
		R8G8B8A8_UNORM,
		R16G16_UNORM,
		R32_FLOAT,
		D32_FLOAT,
		R32_TYPELESS,
		R16_UNORM,
		D16_UNORM,
		R16_TYPELESS,
		R32G32_FLOAT,
		R32G32B32_FLOAT,
		R32G32B32A32_FLOAT,
		R16G16B16A16_FLOAT,
	};

	// TODO: Maybe more generic name for other data types too, ResourceCPUFlags?
	enum class TextureCPUFlags
	{
		NONE,
		READ,
		WRITE,
		READ_WRITE
	};

	enum class TextureWrapMode
	{
		REPEAT,
		CLAMP_EDGE
	};

	enum class TextureFilterMode
	{
		POINT,
		BILINEAR,
		TRILINEAR,
	};

	// @TODO: This is a larger scoped thing for bindable techinqallly
	// @TODO: Bad name !! There is acutaqlly a usage flag in d3d11 but this refers to bindables!!
	// @TODO: Rename this to views/bindables
	enum class TextureUsage
	{
		NONE = 0,
		SHADER_RESOURCE,
		RENDER_TARGET,
		DEPTH_SCENCIL_BUFFER,
		COMPUTER_SHADER_RESOURCE,
		//DEPTH_STENCIL = D3D11_BIND_DEPTH_STENCIL
	};

	enum class VertexShaderLayout
	{
		P,		// @NOTE: Postion
		P_PAD,	// @NOTE: Postion and a padd
		PNT,	// @NOTE: Postions, normal, texture coords(uv)
		PNTM,	// @NOTE: Postions, normal, texture coords(uv), and an instanced model transform matrix
	};

	struct MeshData
	{
		// @TODO: Remove
		CString name;

		ModelId id;

		int32 positionCount;
		Vec3f* positions;
		int32 normalCount;
		Vec3f* normals;
		int32 uvCount;
		Vec2f* uvs;

		// @NOTE: This is not in bytes but in real count
		int32 packedStride;

		int32 packedCount;
		real32* packedVertices;
		int32 indicesCount;
		uint32* indices;

		AABB boundingBox;
	};


	struct ShaderData
	{
		ShaderId id;
		VertexShaderLayout vertexLayout;

		int32 vertexSizeBytes;
		int32 computeSizeBytes;
		int32 pixelSizeBytes;

		// @TODO: Remove this shit !!
		CString name;
		CString vertexPath;
		CString pixelPath;

		union
		{
			char vertexData[Megabytes(1)];
			char computeData[Megabytes(1)];
		};

		char pixelData[Megabytes(1)];
	};

	// @TODO: Textures that are loaded with via the raw loaders are loaded will malloc !!
	struct TextureData
	{
		TextureId id;
		TextureFormat format;
		TextureUsage usage[4];
		TextureCPUFlags cpuFlags;
		int32 width;
		int32 height;
		void* pixels;
		bool32 mips;
	};

	struct AssetState
	{
		int32 textureCount;
		TextureData texturesData[256];

		int32 shaderCount;
		ShaderData shadersData[256];

		int32 meshCount;
		MeshData meshesData[512];
	};

	////////////////////////////////////////////////////
	// @NOTE: Platform layer call this
	////////////////////////////////////////////////////

	void InitializeAssets(AssetState* as);
}