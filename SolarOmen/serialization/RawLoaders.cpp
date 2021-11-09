#include "RawLoaders.h"
#include "RawModelImporter.h"
#include "RawTextureImporter.h"

namespace cm
{
#if USE_RAW_ASSETS
	const CString SHADER_FILE_PATH = "../Assets/Processed/Shaders/";
	const CString MODEL_FILE_PATH = "../Assets/Raw/Models/";
	const CString SKYBOX_FILE_PATH = "../Assets/Raw/Skyboxes/";

	void LoadAllShaders(AssetState* as)
	{
		as->shaderCount = 1;
		as->shadersData[0].name = "INVALID";

		PlatformFolder folder = DEBUGLoadEnitreFolder(SHADER_FILE_PATH, "cso", false);

		int32 processedCount = 0;
		while (processedCount < folder.files.size()) {

			PlatformFile file1 = folder.files[processedCount];

			CString name = Util::StripFilePathAndExtentions(file1.path);
			name.ToUpperCase();
			ShaderId shaderId = ShaderId::ValueOf(name);

			Assert(shaderId != ShaderId::Value::INVALID, "Could not find shader");

			int32 shaderIndex = (int32)shaderId;
			ShaderData* shaderData = &as->shadersData[shaderIndex];
			shaderData->id = shaderId;

			// @NOTE: Compute shader
			if (file1.path.Contains(".comp"))
			{
				shaderData->name = name;

				int32 f1Size = (int32)file1.size_bytes;
				Assert(f1Size <= ArrayCount(shaderData->computeData), "Not enough shader source storage");

				shaderData->computeSizeBytes = f1Size;
				if (f1Size <= ArrayCount(shaderData->computeData))
				{
					memcpy(shaderData->computeData, file1.data, f1Size);
				}

				processedCount++;
				as->shaderCount++;
			}
			else if (file1.path.Contains(".vert") || file1.path.Contains(".pixl"))
			{
				PlatformFile file2 = folder.files[processedCount + 1];

				Assert(file1.path.Contains(".pixl"), "Shader loading err");
				Assert(file2.path.Contains(".vert"), "Shader loading err");

				CString file1Name = file1.path.Split('.').at(0);
				CString file2Name = file2.path.Split('.').at(0);

				Assert(file1Name == file2Name, "Shader loading err");

				shaderData->name = name;

				int32 f1Size = (int32)file1.size_bytes;
				int32 f2Size = (int32)file2.size_bytes;

				shaderData->pixelSizeBytes = f1Size;
				shaderData->vertexSizeBytes = f2Size;

				Assert(f1Size <= ArrayCount(shaderData->pixelData), "Not enough shader source storage");
				Assert(f2Size <= ArrayCount(shaderData->vertexData), "Not enough shader source storage");


				if (f1Size <= ArrayCount(shaderData->pixelData))
				{
					memcpy(shaderData->pixelData, file1.data, f1Size);
				}
				if (f2Size <= ArrayCount(shaderData->vertexData))
				{
					memcpy(shaderData->vertexData, file2.data, f2Size);
				}

				processedCount += 2;
				as->shaderCount++;
			}
			else
			{
				Assert(0, "Shader file error!!");
			}
		}

		DEBUGFreeFolder(&folder);
	}

	static void LoadAllModels(AssetState* as, PlatformFolder folder)
	{
		for (int32 fileIndex = 0; fileIndex < folder.files.size(); fileIndex++)
		{
			PlatformFile file = folder.files[fileIndex];

			CString name = Util::StripFilePathAndExtentions(file.path);
			name.ToUpperCase();

			ModelId id = ModelId::ValueOf(name);
			Assert(id != ModelId::Value::INVALID, "The mesh is not registered with the engine");
			MeshData mesh = LoadModel(file.path);
			mesh.id = id;

			as->meshesData[(uint32)id] = mesh;
			as->meshCount++;
		}
	}

	void LoadAllModels(AssetState* as)
	{
		as->meshCount = 1;
		as->meshesData[0].id = ModelId::Value::INVALID;

		// @TODO: Load screen space quad here
		as->meshCount++;

		// @NOTE: Load all objs
		{
			PlatformFolder folder = DEBUGLoadEnitreFolder(MODEL_FILE_PATH, "obj", true);
			LoadAllModels(as, folder);
			DEBUGFreeFolder(&folder);
		}

		// @NOTE: Load all gltfs
		{
			PlatformFolder folder = DEBUGLoadEnitreFolder(MODEL_FILE_PATH, "glb", true);
			LoadAllModels(as, folder);
			DEBUGFreeFolder(&folder);
		}
	}


	void LoadAllTextures(AssetState* as)
	{
		as->textureCount = 1;
		as->texturesData[0].id = TextureId::Value::INVALID;

		{
			PlatformFolder folder = DEBUGLoadEnitreFolder(MODEL_FILE_PATH, "png", true);

			for (int32 fileIndex = 0; fileIndex < folder.files.size(); fileIndex++)
			{
				PlatformFile file = folder.files[fileIndex];
				CString name = Util::StripFilePathAndExtentions(file.path);
				name.ToUpperCase();

				TextureId id = TextureId::ValueOf(name);
				Assert(id != TextureId::Value::INVALID, "The texture is not registered with the engine");

				TextureData texture = LoadTexture(file.path);
				texture.id = id;
				as->texturesData[(int32)id] = texture;
				as->textureCount++;
			}

			DEBUGFreeFolder(&folder);
		}

		as->skyboxCount = 1;
		as->skyboxes[0].id = SkyboxId::INVALID;

		{
			PlatformFolder folder = DEBUGLoadEnitreFolder(SKYBOX_FILE_PATH, "hdr", true);

			for (int32 fileIndex = 0; fileIndex < folder.files.size(); fileIndex++)
			{
				PlatformFile file = folder.files[fileIndex];
				CString name = Util::StripFilePathAndExtentions(file.path);
				name.ToUpperCase();

				SkyboxId id = GetSkyboxIdFromString(name);
				Assert(id != SkyboxId::INVALID, "The skybox is not registered with the engine");

				SkyboxAsset skybox = LoadSkybox(file.path);
				skybox.id = id;
				as->skyboxes[(int32)id] = skybox;
				as->skyboxCount++;
			}

			DEBUGFreeFolder(&folder);
		}
	}

	void LoadAllFonts(AssetState* as)
	{

	}

	void LoadAllAudio(AssetState* as)
	{

	}
#endif
}