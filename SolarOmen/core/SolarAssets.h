#pragma once
#include "Defines.h"
#include "SolarPlatform.h"
#include "SolarMemory.h"
#include "SolarHashMap.h"
#include "SimpleColliders.h"

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
		R8_BYTE,
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
		TEXT,	// @NOTE: Layout for text rendering
	};

	typedef uint64 AssetId;

	struct ModelAsset
	{
		AssetId id;
		CString name;

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

		//AABB boundingBox;
	};

	struct ShaderAsset
	{
		AssetId id;
		CString name;

		VertexShaderLayout vertexLayout;
		ManagedArray<char> vertexData;
		ManagedArray<char> computeData;
		ManagedArray<char> pixelData;
	};

	// @TODO: Textures that are loaded with via the raw loaders are loaded will malloc !!
	struct TextureAsset
	{
		AssetId id;
		CString name;

		TextureFormat format;
		TextureUsage usage[4];
		TextureCPUFlags cpuFlags;
		int32 width;
		int32 height;
		void* pixels;
		bool32 mips;
	};

	struct FontCharacter
	{
		uint8 character;
		int32 advance;
		Vec2i size;
		Vec2i bearing;
		ManagedArray<uint8> data;
	};

	struct FontAsset
	{
		AssetId id;
		CString name;
		ManagedArray<FontCharacter> chars;
	};

	enum class RoomType
	{
		INVALID = 0,
		MAIN_MENU,
		SINGLE_PLAYER,
		MULTIPLAYER,
		OPTIONS,
		GAME_ROOM,
	};

	struct RoomAsset
	{
		static constexpr uint32 ROOM_HORIZTONAL_SIZE = 255;
		static constexpr uint32 ROOM_VERTICAL_SIZE = 255;

		AssetId id;
		CString name;
		RoomType type;

		bool isTwoPlayerMap;
		Vec3f player1StartPos;
		Vec3f player2StartPos;

		FixedArray<int32, ROOM_HORIZTONAL_SIZE* ROOM_VERTICAL_SIZE> map;
	};

#define GetAssetState() AssetState* as = AssetState::Get()

	class AssetState
	{
	public:
		HashMap<ModelAsset> models;
		HashMap<ShaderAsset> shaders;
		HashMap<TextureAsset> textures;

		FontAsset font;

		inline static void Initialize(AssetState* as) { assetState = as; };
		inline static AssetState* Get() { return assetState; }
	private:
		inline static AssetState* assetState = nullptr;
	};

	template<typename T>
	inline T GetAssetFromName(ManagedArray<T> assetArray, const CString& name)
	{
		for (uint32 i = 0; i < assetArray.GetCount(); i++)
		{
			if (assetArray[i].name == name)
				return assetArray[i];
		}

		Assert(0, "Could not find asset");

		return {};
	}

	namespace Assets
	{
		bool32 Initialize();
		void Shutdown();
	}
}