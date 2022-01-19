#include "SolarResourceTypes.h"
#include "platform/SolarPlatform.h"
#include "core/SolarLogging.h"

namespace sol
{
	static HashMap<ProgramResource>* programs;
	static HashMap<ModelResource>* models;
	static HashMap<TextureResource>* textures;

	static String PACKED_ASSET_PATH = "Assets/Packed/";
	bool8 Resources::Initialize()
	{
		programs = GameMemory::PushPermanentStruct<HashMap<ProgramResource>>();
		models = GameMemory::PushPermanentStruct<HashMap<ModelResource>>();
		textures = GameMemory::PushPermanentStruct<HashMap<TextureResource>>();

		bool8 modelsLoaded = LoadAllModelResources();
		bool8 texturesLoaded = LoadAllTextureResources();
		bool8 programsLoaded = LoadAllProgramResources();

		return modelsLoaded && texturesLoaded && programsLoaded;
	}

	void Resources::Shutdown()
	{
		models->Clear();
		textures->Clear();
		programs->Clear();
	}

	template<typename T>
	inline T* GetResourcesFromName(ManagedArray<T> assetArray, const String& name)
	{
		for (uint32 i = 0; i < assetArray.GetCount(); i++)
		{
			if (assetArray[i].name == name) { return assetArray.Get(i); }
		}

		SOLFATAL(String("Could not find resource: ").Add(name).GetCStr());
		Assert(0, "Could not find asset");
		return {};
	}

	ManagedArray<ProgramResource> Resources::GetAllProgramResources()
	{
		return programs->GetValueSet();
	}

	ProgramResource* Resources::GetProgramResource(const String& name)
	{
		return GetResourcesFromName<ProgramResource>(programs->GetValueSet(), name);
	}

	ManagedArray<ModelResource> Resources::GetAllModelResources()
	{
		return models->GetValueSet();
	}

	ModelResource* Resources::GetModelResource(const ResourceId& id)
	{
		return models->Get(id);
	}

	ModelResource* Resources::GetModelResource(const String& name)
	{
		return GetResourcesFromName<ModelResource>(models->GetValueSet(), name);
	}

	ManagedArray<TextureResource> Resources::GetAllTextureResources()
	{
		return textures->GetValueSet();
	}

	TextureResource* Resources::GetTextureResource(const String& name)
	{
		return GetResourcesFromName<TextureResource>(textures->GetValueSet(), name);
	}

	TextureResource* Resources::GetTextureResource(const ResourceId& id)
	{
		return textures->Get(id);
	}

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
		inline String Read<String>()
		{
			Assert(cursor < file.sizeBytes, "BinaryAssetFile");
			int32 length = Read<int32>();

			String result = "";
			for (int32 i = 0; i < length; i++)
			{
				char c = Read<char>();
				result.Add(c);
			}

			return result;
		}
	};

	void CreateBuiltInModels()
	{
		ModelResource* invalid = models->Create(0);
		invalid->name = "NONE/INVALID";
		invalid->id.number = 0;

		ModelResource* plane = models->Create(1);
		plane->name = "Plane";
		plane->id.number = 1;
		plane->meshes.Allocate(1, MemoryType::PERMANENT);
		plane->meshes.Add(ModelGenerator::CreateQuad(-1, 1, 2, 2, 0));

		ModelResource* cube = models->Create(2);
		cube->name = "Cube";
		cube->id.number = 2;
		cube->meshes.Allocate(1, MemoryType::PERMANENT);
		cube->meshes.Add(ModelGenerator::CreateBox(1, 1, 1, 1, VertexLayoutType::Value::PNT));
	}

	bool8 Resources::LoadAllModelResources()
	{
		models->Clear();
		CreateBuiltInModels();

		return 1;

		BinaryAssetFile file = {};
		file.file = Platform::LoadEntireFile(String(PACKED_ASSET_PATH).Add("models.bin"), false);

		if (file.file.data)
		{
			uint32 modelCount = file.Read<uint32>();
			for (uint32 modelIndex = 0; modelIndex < modelCount; modelIndex++)
			{
				ResourceId modelId = file.Read<ResourceId>();
				ModelResource* model = models->Create(modelId.number);
				model->id = modelId;
				model->name = file.Read<String>();

				uint32 meshCount = file.Read<uint32>();
				model->meshes.Allocate(meshCount, MemoryType::PERMANENT);
				model->textures.Allocate(meshCount, MemoryType::PERMANENT);

				for (uint32 meshIndex = 0; meshIndex < meshCount; meshIndex++)
				{
					MeshResource mesh = {};
					mesh.name = file.Read<String>();

					model->textures[meshIndex].abledoTexture = file.Read<ResourceId>();
					model->textures[meshIndex].occlusionRoughnessMetallicTexture = file.Read<ResourceId>();
					model->textures[meshIndex].normalTexture = file.Read<ResourceId>();
					model->textures[meshIndex].emssiveTexture = file.Read<ResourceId>();

					mesh.layout = file.Read<VertexLayoutType::Value>();

					mesh.packedVertices.Allocate(file.Read<uint32>() * mesh.layout.GetStride(), MemoryType::PERMANENT);
					for (uint32 i = 0; i < mesh.packedVertices.GetCapcity(); i++)
					{
						mesh.packedVertices.Add(file.Read<real32>());
					}

					mesh.indices.Allocate(file.Read<uint32>(), MemoryType::PERMANENT);
					for (uint32 i = 0; i < mesh.indices.GetCapcity(); i++)
					{
						mesh.indices.Add(file.Read<uint32>());
					}

					model->meshes.Add(mesh);
				}

				SOLTRACE(String("Loaded model resource: ").Add(model->name).GetCStr());
			}

			SOLINFO("Model loading complete");
			return true;
		}
		else
		{
			SOLFATAL("Could not find/open model asset file");
		}

		return false;
	}

	bool8 Resources::LoadAllTextureResources()
	{
		textures->Clear();

		textures->Create(0)->name = "NONE/INVALID";
		return 1;

		BinaryAssetFile file = {};
		file.file = Platform::LoadEntireFile(String(PACKED_ASSET_PATH).Add("textures.bin"), false);

		if (file.file.data)
		{
			uint32 textureCount = file.Read<uint32>();
			for (uint32 textureIndex = 0; textureIndex < textureCount; textureIndex++)
			{
				TextureResource texture = {};
				texture.id = file.Read<ResourceId>();
				texture.name = file.Read<String>();
				texture.mips = (bool32)file.Read<uint8>();
				texture.width = file.Read<uint32>();
				texture.height = file.Read<uint32>();
				texture.format = file.Read<TextureFormat::Value>();
				texture.usage[0] = file.Read<BindUsage::Value>();
				texture.usage[1] = file.Read<BindUsage::Value>();
				texture.usage[2] = file.Read<BindUsage::Value>();
				texture.usage[3] = file.Read<BindUsage::Value>();
				texture.cpuFlags = file.Read<ResourceCPUFlags::Value>();

				uint32 pixelCount = file.Read<uint32>();
				texture.pixels.Allocate(pixelCount, MemoryType::PERMANENT);
				for (uint32 i = 0; i < pixelCount; i++) { texture.pixels.Add(file.Read<uint8>()); }

				SOLTRACE(String("Loaded texture resource: ").Add(texture.name).GetCStr());

				textures->Put(texture.id.number, texture);
			}

			SOLINFO("Texture loading complete");
			return true;
		}
		else
		{
			SOLFATAL("Could not find/open texture asset file");
		}

		return false;
	}

	bool8 Resources::LoadAllProgramResources()
	{
		programs->Clear();

		programs->Create(0)->name = "NONE/INVALID";

		BinaryAssetFile file = {};
		file.file = Platform::LoadEntireFile(String(PACKED_ASSET_PATH).Add("programs.bin"), false);

		if (file.file.data)
		{

			uint32 programCount = file.Read<uint32>();
			for (uint32 programIndex = 0; programIndex < programCount; programIndex++)
			{
				ProgramResource shader = {};
				shader.id = file.Read<ResourceId>();
				shader.name = file.Read<String>();
				shader.stageLayout = file.Read<ProgramStagesLayout::Value>();
				shader.vertexLayout = file.Read<VertexLayoutType::Value>();

				switch (shader.stageLayout.Get())
				{
				case ProgramStagesLayout::Value::VERTEX_PIXEL:
				{
					shader.vertexData.Allocate(file.Read<uint32>(), MemoryType::PERMANENT);
					for (uint32 i = 0; i < shader.vertexData.GetCapcity(); i++) { shader.vertexData.Add(file.Read<uint8>()); }
					shader.pixelData.Allocate(file.Read<uint32>(), MemoryType::PERMANENT);
					for (uint32 i = 0; i < shader.pixelData.GetCapcity(); i++) { shader.pixelData.Add(file.Read<uint8>()); }

				} break;
				case ProgramStagesLayout::Value::COMPUTE:
				{
					shader.computeData.Allocate(file.Read<uint32>(), MemoryType::PERMANENT);
					for (uint32 i = 0; i < shader.computeData.GetCapcity(); i++) { shader.computeData.Add(file.Read<uint8>()); }
				} break;
				default: Assert(0, "Loading shaders");
				}

				SOLTRACE(String("Loaded shader resource: ").Add(shader.name).GetCStr());
				programs->Put(shader.id.number, shader);
			}

			SOLINFO("Shader loading complete");
			return true;
		}
		else
		{
			SOLFATAL("Could not find/open shader programs asset file");
		}

		return false;
	}
}