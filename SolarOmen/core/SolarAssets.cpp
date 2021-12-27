#include "SolarAssets.h"
#include "SolarDebug.h"

#include <random>

#if USE_RAW_ASSETS
#include "serialization/RawModelImporter.h"
#include "serialization/RawTextureImporter.h"
#include "serialization/RawShaderImporter.h"
#include "serialization/RawFontImporter.h"
#include "serialization/RawRoomImporter.h"
#include "serialization/RawAudioImporter.h"
#else

#endif 

namespace cm
{
	//void AssetState::Initialize(AssetState* as)
	//{
	//	/*assetState = as;
	//	LoadAllShaders(as);
	//	LoadAllModels(as);
	//	LoadAllTextures(as);
	//	LoadAllFonts(as);
	//	LoadAllAudio(as);*/
	//}

	inline static AssetState* as = nullptr;
	static std::random_device randomDevice;
	static std::mt19937_64 randomEngine(randomDevice());
	static std::uniform_int_distribution<uint64> randomDistribution;

	static CString ASSET_PATH = "F:/codes/SolarOmen/SolarOmen-2/Assets/Raw/";

	inline uint64 GenerateGUID()
	{
		return randomDistribution(randomEngine);
	}

	int32 FindMetaFile(ManagedArray<CString> metaFiles, const CString& other)
	{
		CString test = Util::StripFilePathAndExtentions(other);
		for (int32 i = 0; i < (int32)metaFiles.GetCount(); i++)
		{
			CString meta = Util::StripFilePathAndExtentions(metaFiles[i]);
			if (test == meta)
				return i;
		}

		return -1;
	}

	CString ParseLine(const char* data, int32* current)
	{
		CString result;
		if (data)
		{
			for (;; (*current)++)
			{
				if (data[*current] == '\0')
				{
					break;
				}

				if (data[*current] == '\n')
				{
					(*current)++;
					break;
				}
				result.Add(data[*current]);
			}
		}

		return result;
	}

	ManagedArray<ModelAsset> LoadOrCreateModelMetaData(ManagedArray<CString> metaFiles, ManagedArray<CString> modelFiles)
	{
		ManagedArray<ModelAsset> models = {};
		models.data = GameMemory::PushTransientCount<ModelAsset>(modelFiles.GetCount());
		models.capcity = modelFiles.GetCount();

		for (uint32 i = 0; i < modelFiles.GetCount(); i++)
		{
			int32 index = FindMetaFile(metaFiles, modelFiles[i]);
			uint64 guid = 0;

			if (index >= 0)
			{
				PlatformFile metaFile = Platform::LoadEntireFile(metaFiles[index], false);

				int32 current = 0;
				CString line = ParseLine((const char*)metaFile.data, &current);
				bool32 correctFile = false;
				while (line.GetLength() > 0)
				{
					if (line.StartsWith("GUID="))
					{
						guid = line.Split('=')[1].ToUint64();
					}

					if (line == "Type=Model")
					{
						correctFile = true;
					}
					line = ParseLine((const char*)metaFile.data, &current);
				}

				if (!correctFile)
				{
					Debug::LogInfo("Incorrect meta file !!");
					continue;
				}
			}
			else
			{
				CString data = "";
				guid = randomDistribution(randomEngine);
				data.Add("GUID=").Add(guid).Add("\n");
				data.Add("Type=Model\n");

				CString path = Util::StripFileExtension(modelFiles[i]);
				path.Add(".slo");

				Debug::LogInfo(CString("No meta file for ").Add(modelFiles[i]).Add(" creating one ").Add(guid));
				if (!Platform::WriteFile(path, (void*)data.GetCStr(), data.GetLength()))
				{
					Debug::LogInfo("Could not save meta file !!");
				}
			}

			ModelAsset asset = {};
			asset.id = guid;
			asset.name = Util::StripFilePathAndExtentions(modelFiles[i]);
			models.Add(asset);
		}

		return models;
	}

	ManagedArray<TextureAsset> LoadOrCreateTextureMetaData(ManagedArray<CString> metaFiles, ManagedArray<CString> textureFiles)
	{
		ManagedArray<TextureAsset> textures = {};
		textures.data = GameMemory::PushTransientCount<TextureAsset>(textureFiles.GetCount());
		textures.capcity = textureFiles.GetCount();

		for (uint32 i = 0; i < textureFiles.GetCount(); i++)
		{
			int32 index = FindMetaFile(metaFiles, textureFiles[i]);
			uint64 guid = 0;

			if (index >= 0)
			{
				PlatformFile metaFile = Platform::LoadEntireFile(metaFiles[index], false);

				int32 current = 0;
				CString line = ParseLine((const char*)metaFile.data, &current);
				bool32 correctFile = false;

				while (line.GetLength() > 0)
				{
					if (line.StartsWith("GUID="))
					{
						guid = line.Split('=')[1].ToUint64();
					}

					if (line == "Type=Texture")
					{
						correctFile = true;
					}
					line = ParseLine((const char*)metaFile.data, &current);
				}

				if (!correctFile)
				{
					Debug::LogInfo(CString("Incorrect meta file for: ").Add(textureFiles[i]));
					continue;
				}
			}
			else
			{
				CString data = "";
				guid = randomDistribution(randomEngine);
				data.Add("GUID=").Add(guid).Add("\n");
				data.Add("Type=Texture\n");

				Debug::LogInfo(CString("No meta file for ").Add(textureFiles[i]).Add(" creating one ").Add(guid));

				CString path = Util::StripFileExtension(textureFiles[i]);
				path.Add(".slo");

				if (!Platform::WriteFile(path, (void*)data.GetCStr(), data.GetLength()))
				{
					Debug::LogInfo("Could not save meta file !!");
				}
			}

			TextureAsset asset = {};
			asset.id = guid;
			asset.name = Util::StripFilePathAndExtentions(textureFiles[i]);
			textures.Add(asset);
		}

		return textures;
	}

	bool32 Assets::Initialize()
	{
		as = GameMemory::PushPermanentStruct<AssetState>();
		AssetState::Initialize(as);

		ManagedArray<CString> metaFiles = Platform::LoadEntireFolder(ASSET_PATH, "slo");
		ManagedArray<CString> modelFiles = Platform::LoadEntireFolder(ASSET_PATH, "obj");
		ManagedArray<CString> textureFiles = Platform::LoadEntireFolder(ASSET_PATH, "png");

		ManagedArray<CString> vertexShaderFiles = Platform::LoadEntireFolder(ASSET_PATH, "vert.cso");
		ManagedArray<CString> pixelShaderFiles = Platform::LoadEntireFolder(ASSET_PATH, "pixl.cso");

		ManagedArray<CString> roomFiles = Platform::LoadEntireFolder(CString(ASSET_PATH).Add("Rooms/"), "txt");

		ManagedArray<CString> audioFiles = Platform::LoadEntireFolder(CString(ASSET_PATH), "wav");

		ManagedArray<ModelAsset> models = LoadOrCreateModelMetaData(metaFiles, modelFiles);
		ManagedArray<TextureAsset> textures = LoadOrCreateTextureMetaData(metaFiles, textureFiles);

		for (uint32 modelIndex = 0; modelIndex < modelFiles.GetCount(); modelIndex++)
		{
			ModelAsset modelAsset = LoadModel(modelFiles[modelIndex]);

			// @NOTE: Copy over the meta data
			modelAsset.name = models[modelIndex].name;
			modelAsset.id = models[modelIndex].id;

			// @NOTE: Now override
			models[modelIndex] = modelAsset;
			as->models.Put(modelAsset.id, modelAsset);
		}

		for (uint32 textureIndex = 0; textureIndex < textureFiles.GetCount(); textureIndex++)
		{
			TextureAsset textureAsset = LoadTexture(textureFiles[textureIndex]);

			// @NOTE: Copy over the meta data
			textureAsset.name = textures[textureIndex].name;
			textureAsset.id = textures[textureIndex].id;

			// @NOTE: Now override
			textures[textureIndex] = textureAsset;
			as->textures.Put(textureAsset.id, textureAsset);
		}

		Assert(vertexShaderFiles.GetCount() == pixelShaderFiles.GetCount(), "Vertex shader does not have pixel shader or vice versa");
		for (uint32 shaderIndex = 0; shaderIndex < vertexShaderFiles.GetCount(); shaderIndex++)
		{
			CString vertexPath = vertexShaderFiles[shaderIndex];
			CString pixelPath = pixelShaderFiles[shaderIndex];

			Assert(Util::StripFilePathAndExtentions(vertexPath) == Util::StripFilePathAndExtentions(pixelPath),
				"Vertex shader does not have pixel shader or vice versa");

			ShaderAsset shaderAsset = LoadShader(vertexPath, pixelPath);
			shaderAsset.id = GenerateGUID();
			shaderAsset.name = Util::StripFilePathAndExtentions(vertexPath);

			as->shaders.Put(shaderAsset.id, shaderAsset);
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
}
