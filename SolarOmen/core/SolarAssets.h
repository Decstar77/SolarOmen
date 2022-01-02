#pragma once
#include "Defines.h"
#include "SolarPlatform.h"
#include "SolarMemory.h"
#include "SolarContainers.h"
#include "SimpleColliders.h"
#include "SolarTypes.h"
#include "SolarEntity.h"

namespace cm
{
	struct UnpackedModelAsset
	{
		CString name;
		uint32 vertexCount;
		uint32 triangleCount;
		ManagedArray<uint32> indices;
		ManagedArray<Vec3f> positions;
		ManagedArray<Vec3f> normals;
		ManagedArray<Vec2f> uvs;
		ManagedArray<Vec3f> tangents;
		ManagedArray<Vec3f> bitangents;
		ManagedArray<Vec4f> colours;

		inline Triangle GetTriangle(uint32 triangleIndex)
		{
			uint32 vertexIndex = triangleIndex * 3;

			uint32 i0 = indices[vertexIndex];
			uint32 i1 = indices[vertexIndex + 1];
			uint32 i2 = indices[vertexIndex + 2];

			Triangle tri = {};
			tri.v0 = positions[i0];
			tri.v1 = positions[i1];
			tri.v2 = positions[i2];

			return tri;
		}
	};

	struct ModelAsset
	{
		AssetId id;
		CString name;

		VertexLayoutType layout;
		ManagedArray<real32> packedVertices;
		ManagedArray<uint32> indices;

		AABB boundingBox;

		// @NOTE: Assumes the model is trianglulated !!
		UnpackedModelAsset Unpack();
	};

	struct ShaderAsset
	{
		AssetId id;
		CString name;
		ProgramStagesLayout stageLayout;
		VertexLayoutType vertexLayout;
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
		BindUsage usage[4];
		ResourceCPUFlags cpuFlags;
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

		real32 GetWidthOfText(const CString& text, real32 scale);
		real32 GetHeightOfText(const CString& text, real32 scale);
	};

	enum class RoomType
	{
		INVALID = 0,
		MAIN_MENU,
		LEVEL_SELECTION,
		MULTIPLAYER,
		OPTIONS,
		LEVEL_1,
		LEVEL_2,
		LEVEL_3,
		LEVEL_4,
		LEVEL_5,
		LEVEL_6,
		LEVEL_7,
		LEVEL_8,
		LEVEL_9,
		LEVEL_10,
		LEVEL_11,
		LEVEL_12,
		LEVEL_13,
		LEVEL_14,
		LEVEL_15,
		LEVEL_16,
		LEVEL_17,
		LEVEL_18,
		LEVEL_19,
		LEVEL_20,
		COUNT
	};

	struct EntityAsset
	{
		EntityId id;
		CString name;
		Tag tag;
		Transform localTransform;
		RenderComponent renderComponent;
		ColliderComponent colliderComponent;
		BrainComponent brainComponent;
	};

	struct RoomAsset
	{
		static constexpr uint32 ROOM_HORIZTONAL_SIZE = 255;
		static constexpr uint32 ROOM_VERTICAL_SIZE = 255;

		CString name;
		RoomType type;

		bool isTwoPlayerMap;
		Vec3f player1StartPos;
		Vec3f player2StartPos;

		FixedArray<EntityAsset, 1024> entities;
	};

	struct MaterialAsset
	{
		AssetId id;
		CString name;

		Vec3f colourKd;
	};

#define GetAssetState() AssetState* as = AssetState::Get()

	class AssetState
	{
	public:
		HashMap<ModelAsset> models;
		HashMap<ShaderAsset> shaders;
		HashMap<TextureAsset> textures;
		HashMap<MaterialAsset> materials;


		FixedArray<RoomAsset, (uint32)RoomType::COUNT> rooms;
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