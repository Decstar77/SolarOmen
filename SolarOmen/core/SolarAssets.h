#pragma once
#include "Defines.h"
#include "SolarPlatform.h"
#include "SolarMemory.h"
#include "SolarContainers.h"
#include "SimpleColliders.h"

#include "SolarEntity.h"

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

	class VertexShaderLayoutType
	{
	public:
		enum class Value : uint32
		{
			INAVLID = 0,
			P,		// @NOTE: Postion
			P_PAD,	// @NOTE: Postion and a padd
			PNT,	// @NOTE: Postions, normal, texture coords(uv)
			PNTC,   // @NOTE: Postions, normal, texture coords(uv), vertex colour
			PNTM,	// @NOTE: Postions, normal, texture coords(uv), and an instanced model transform matrix
			TEXT,	// @NOTE: Layout for text rendering
			COUNT,
		};

		VertexShaderLayoutType()
		{
			value = Value::INAVLID;
		}

		VertexShaderLayoutType(Value v)
		{
			this->value = v;
		}

		inline CString ToString() const
		{
			CString copy = __STRINGS__[(uint32)value];

			return copy;
		}

		inline uint32 GetStride() const
		{
			switch (value)
			{
			case Value::P: return 1;
			case Value::P_PAD: return 2;
			case Value::PNT: return 3 + 3 + 2;
			case Value::PNTC: return 3 + 3 + 2 + 4;
			case Value::PNTM: return 3 + 3 + 2 + 16;
			case Value::TEXT: return 4;
			}

			Assert(0, "INVALID STRIDE");
			return 0;
		}

		inline Value Get() const { return value; }

		inline static VertexShaderLayoutType ValueOf(const uint32& v)
		{
			Assert(v < (uint32)Value::COUNT, "Invalid model id");
			return (VertexShaderLayoutType::Value)v;
		}

		inline static VertexShaderLayoutType ValueOf(const CString& str)
		{
			uint32 count = (uint32)Value::COUNT;
			for (uint32 i = 0; i < count; i++)
			{
				if (str == __STRINGS__[i])
				{
					return ValueOf(i);
				}
			}

			return Value::INAVLID;
		}

		inline bool operator==(const VertexShaderLayoutType& rhs) const
		{
			return this->value == rhs.value;
		}

		inline bool operator!=(const VertexShaderLayoutType& rhs) const
		{
			return this->value != rhs.value;
		}

	private:
		Value value;

		inline static const CString __STRINGS__[] = {
			"INAVLID",
			"P",
			"P_PAD",
			"PNT",
			"PNTC",
			"PNTM",
			"TEXT",
			"COUNT",
		};
	};

	struct FatVertex
	{
		inline static constexpr uint32 MAX_BONE_INFLUENCE = 4;

		Vec3f position;
		Vec3f normal;
		Vec2f texCoords;
		Vec3f tangent;
		Vec3f bitangent;
		Vec4f colours;
		int32 boneIds[MAX_BONE_INFLUENCE] = {};
		real32 boneWeights[MAX_BONE_INFLUENCE] = {};
	};

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

		VertexShaderLayoutType layout;
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

		VertexShaderLayoutType vertexLayout;
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