#include "SolarAssets.h"
#include "SolarDebug.h"

#include <random>

#if USE_RAW_ASSETS
#include "serialization/RawFontImporter.h"
#include "serialization/RawRoomImporter.h"
#include "serialization/RawAudioImporter.h"
#else

#endif 

namespace cm
{
	inline static AssetState* as = nullptr;
	static CString ASSET_PATH = "F:/codes/SolarOmen/SolarOmen-2/Assets/Raw/";
	static CString PACKED_ASSET_PATH = "F:/codes/SolarOmen/SolarOmen-2/Assets/Packed/";

	struct BinaryAssetFile
	{
		uint64 cursor;
		PlatformFile file;

		template<typename T>
		inline T Read()
		{
			Assert(cursor < file.sizeBytes, "BinaryAssetFile");
			uint64 index = cursor;
			cursor += sizeof(T);
			return *(T*)(&((char*)(file.data))[index]);
		}

		template<>
		inline CString Read<CString>()
		{
			Assert(cursor < file.sizeBytes, "BinaryAssetFile");
			int32 length = Read<int32>();

			CString result = "";
			for (int32 i = 0; i < length; i++)
			{
				char c = Read<char>();
				result.Add(c);
			}

			return result;
		}
	};

	bool32 Assets::Initialize()
	{
		as = GameMemory::PushPermanentStruct<AssetState>();
		AssetState::Initialize(as);

		ManagedArray<CString> roomFiles = Platform::LoadEntireFolder(CString(ASSET_PATH).Add("Rooms/"), "txt");

		ManagedArray<CString> audioFiles = Platform::LoadEntireFolder(CString(ASSET_PATH), "wav");

		{
			BinaryAssetFile binModels = {};
			binModels.file = Platform::LoadEntireFile(CString(PACKED_ASSET_PATH).Add("models.bin"), false);

			uint32 modelCount = binModels.Read<uint32>();
			for (uint32 modelIndex = 0; modelIndex < modelCount; modelIndex++)
			{
				ModelAsset model = {};
				model.id = binModels.Read<AssetId>();
				model.name = binModels.Read<CString>();
				model.layout = binModels.Read<VertexLayoutType::Value>();

				model.packedVertices.Allocate(binModels.Read<uint32>() * model.layout.GetStride(), MemoryType::PERMANENT);
				for (uint32 i = 0; i < model.packedVertices.GetCapcity(); i++)
				{
					model.packedVertices.Add(binModels.Read<real32>());
				}

				model.indices.Allocate(binModels.Read<uint32>(), MemoryType::PERMANENT);
				for (uint32 i = 0; i < model.indices.GetCapcity(); i++)
				{
					model.indices.Add(binModels.Read<uint32>());
				}

				as->models.Put(model.id.number, model);
			}
		}
		{
			BinaryAssetFile binTextures = {};
			binTextures.file = Platform::LoadEntireFile(CString(PACKED_ASSET_PATH).Add("textures.bin"), false);

			uint32 textureCount = binTextures.Read<uint32>();
			for (uint32 textureIndex = 0; textureIndex < textureCount; textureIndex++)
			{
				TextureAsset texture = {};
				texture.id = binTextures.Read<AssetId>();
				texture.name = binTextures.Read<CString>();
				texture.mips = (bool32)binTextures.Read<uint8>();
				texture.width = binTextures.Read<uint32>();
				texture.height = binTextures.Read<uint32>();
				texture.format = binTextures.Read<TextureFormat::Value>();
				texture.usage[0] = binTextures.Read<BindUsage::Value>();
				texture.usage[1] = binTextures.Read<BindUsage::Value>();
				texture.usage[2] = binTextures.Read<BindUsage::Value>();
				texture.usage[3] = binTextures.Read<BindUsage::Value>();
				texture.cpuFlags = binTextures.Read<ResourceCPUFlags::Value>();

				uint32 pixelCount = binTextures.Read<uint32>();
				uint8* pixels = GameMemory::PushPermanentCount<uint8>(pixelCount);

				for (uint32 i = 0; i < pixelCount; i++)
				{
					pixels[i] = binTextures.Read<uint8>();
				}

				texture.pixels = pixels;

				as->textures.Put(texture.id.number, texture);
			}
		}
		{
			BinaryAssetFile binPrograms = {};
			binPrograms.file = Platform::LoadEntireFile(CString(PACKED_ASSET_PATH).Add("programs.bin"), false);

			uint32 programCount = binPrograms.Read<uint32>();
			for (uint32 programIndex = 0; programIndex < programCount; programIndex++)
			{
				ShaderAsset shader = {};
				shader.id = binPrograms.Read<AssetId>();
				shader.name = binPrograms.Read<CString>();
				shader.stageLayout = binPrograms.Read<ProgramStagesLayout::Value>();
				shader.vertexLayout = binPrograms.Read<VertexLayoutType::Value>();

				switch (shader.stageLayout.Get())
				{
				case ProgramStagesLayout::Value::VERTEX_PIXEL:
				{
					shader.vertexData.Allocate(binPrograms.Read<uint32>(), MemoryType::PERMANENT);
					for (uint32 i = 0; i < shader.vertexData.GetCapcity(); i++) { shader.vertexData.Add(binPrograms.Read<uint8>()); }
					shader.pixelData.Allocate(binPrograms.Read<uint32>(), MemoryType::PERMANENT);
					for (uint32 i = 0; i < shader.pixelData.GetCapcity(); i++) { shader.pixelData.Add(binPrograms.Read<uint8>()); }

				} break;
				case ProgramStagesLayout::Value::COMPUTE:
				{
					shader.computeData.Allocate(binPrograms.Read<uint32>(), MemoryType::PERMANENT);
					for (uint32 i = 0; i < shader.computeData.GetCapcity(); i++) { shader.computeData.Add(binPrograms.Read<uint8>()); }
				} break;
				default: Assert(0, "Loading shaders");
				}

				as->shaders.Put(shader.id.number, shader);
			}
		}

		for (uint32 roomIndex = 0; roomIndex < roomFiles.GetCount(); roomIndex++)
		{
			RoomAsset* room = LoadRoom(roomFiles[roomIndex]);
			as->rooms.Add(*room);
		}

		for (uint32 audioIndex = 0; audioIndex < audioFiles.GetCount(); audioIndex++)
		{
			CString name = LoadAudio(audioFiles[audioIndex]);
		}

		as->font = LoadFont(CString(ASSET_PATH).Add("Fonts/Grind-regular.otf"));

		GameMemory::ReleaseAllTransientMemory();

		return true;
	}

	void Assets::Shutdown()
	{

	}


	real32 FontAsset::GetWidthOfText(const CString& text, real32 scale)
	{
		real32 width = 0;
		for (int32 i = 0; i < text.GetLength(); i++)
		{
			FontCharacter ch = {};
			for (uint32 j = 0; j < chars.count; j++)
			{
				if (text[i] == chars[j].character)
				{
					ch = chars[j];
					break;
				}
			}

			width += (ch.advance >> 6) * scale;
		}

		return width;
	}

	real32 FontAsset::GetHeightOfText(const CString& text, real32 scale)
	{
		real32 maxY = REAL_MIN;

		for (int32 i = 0; i < text.GetLength(); i++)
		{
			FontCharacter ch = {};
			for (uint32 j = 0; j < chars.count; j++)
			{
				if (text[i] == chars[j].character)
				{
					ch = chars[j];
					break;
				}
			}

			real32 y = ch.size.y * scale + (ch.size.y - ch.bearing.y) * scale;

			maxY = Max(maxY, y);
		}

		return maxY;
	}

	UnpackedModelAsset ModelAsset::Unpack()
	{
		UnpackedModelAsset result = {};
		result.indices.Allocate(indices.count, MemoryType::TRANSIENT);
		for (uint32 i = 0; i < indices.count; i++)
		{
			uint32 vertexIndex = indices[i];
			result.indices.Add(vertexIndex);
		}

		result.vertexCount = packedVertices.count / layout.GetStride();
		result.triangleCount = result.vertexCount / 3;
		result.name = name;

		switch (layout.Get())
		{
		case VertexLayoutType::Value::P:Assert(0, "Can't unpack model layout"); break;
		case VertexLayoutType::Value::P_PAD: Assert(0, "Can't unpack model layout"); break;
		case VertexLayoutType::Value::PNT:
		{
			result.positions.Allocate(result.vertexCount, MemoryType::TRANSIENT);
			result.normals.Allocate(result.vertexCount, MemoryType::TRANSIENT);
			result.uvs.Allocate(result.vertexCount, MemoryType::TRANSIENT);

			for (uint32 index = 0, i = 0; i < packedVertices.count; i += layout.GetStride(), index++)
			{
				result.positions[index] = Vec3f(packedVertices[i], packedVertices[i + 1], packedVertices[i + 2]);
				result.normals[index] = Vec3f(packedVertices[i + 3], packedVertices[i + 4], packedVertices[i + 5]);
				result.uvs[index] = Vec2f(packedVertices[i + 6], packedVertices[i + 7]);
			}
		} break;
		case VertexLayoutType::Value::PNTC:
		{
			result.positions.Allocate(result.vertexCount, MemoryType::TRANSIENT);
			result.normals.Allocate(result.vertexCount, MemoryType::TRANSIENT);
			result.uvs.Allocate(result.vertexCount, MemoryType::TRANSIENT);
			result.colours.Allocate(result.vertexCount, MemoryType::TRANSIENT);

			for (uint32 index = 0, i = 0; i < packedVertices.count; i += layout.GetStride(), index++)
			{
				result.positions[index] = Vec3f(packedVertices[i], packedVertices[i + 1], packedVertices[i + 2]);
				result.normals[index] = Vec3f(packedVertices[i + 3], packedVertices[i + 4], packedVertices[i + 5]);
				result.uvs[index] = Vec2f(packedVertices[i + 6], packedVertices[i + 7]);
				result.colours[index] = Vec4f(packedVertices[i + 8], packedVertices[i + 9], packedVertices[i + 10], packedVertices[i + 11]);
			}
		} break;
		case VertexLayoutType::Value::PNTM:Assert(0, "Can't unpack model layout"); break;
		case VertexLayoutType::Value::TEXT:Assert(0, "Can't unpack model layout"); break;
		case VertexLayoutType::Value::INVALID:Assert(0, "Can't unpack model layout"); break;
		case VertexLayoutType::Value::COUNT:Assert(0, "Can't unpack model layout"); break;
		}

		return result;
	}
}
