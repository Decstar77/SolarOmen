#include "SolarResources.h"
#include "platform/SolarPlatform.h"
#include "core/SolarLogging.h"

namespace sol
{
	static HashMap<ProgramResource> programs = {};
	static HashMap<ModelResource> models = {};
	static HashMap<TextureResource> textures = {};

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
		return programs.GetValueSet();
	}

	ProgramResource* Resources::GetProgramResource(const String& name)
	{
		return GetResourcesFromName<ProgramResource>(programs.GetValueSet(), name);
	}

	ManagedArray<ModelResource> Resources::GetAllModelResources()
	{
		return models.GetValueSet();
	}

	ModelResource* Resources::GetModelResource(const ResourceId& id)
	{
		return models.Get(id);
	}

	ModelResource* Resources::GetModelResource(const String& name)
	{
		return GetResourcesFromName<ModelResource>(models.GetValueSet(), name);
	}

	ManagedArray<TextureResource> Resources::GetAllTextureResources()
	{
		return textures.GetValueSet();
	}

	TextureResource* Resources::GetTextureResource(const String& name)
	{
		return GetResourcesFromName<TextureResource>(textures.GetValueSet(), name);
	}

	TextureResource* Resources::GetTextureResource(const ResourceId& id)
	{
		return textures.Get(id);
	}

	static String PACKED_ASSET_PATH = "F:/codes/SolarOmen/SolarOmen-2/Assets/Packed/";
	bool8 ResourceSystem::Initialize()
	{
		LoadAllModels();
		LoadAllTextures();
		LoadAllShaderPrograms();
		return true;
	}

	void ResourceSystem::Shutdown()
	{
		programs.Clear();
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

	void ResourceSystem::LoadAllModels()
	{
		ModelResource invalid = {};
		invalid.name = "NONE/INVALID";
		models.Put(0, invalid);

		BinaryAssetFile file = {};
		file.file = Platform::LoadEntireFile(String(PACKED_ASSET_PATH).Add("models.bin"), false);

		uint32 modelCount = file.Read<uint32>();
		for (uint32 modelIndex = 0; modelIndex < modelCount; modelIndex++)
		{
			ModelResource model = {};
			model.id = file.Read<ResourceId>();
			model.name = file.Read<String>();
			model.layout = file.Read<VertexLayoutType::Value>();

			model.packedVertices.Allocate(file.Read<uint32>() * model.layout.GetStride(), MemoryType::PERMANENT);
			for (uint32 i = 0; i < model.packedVertices.GetCapcity(); i++)
			{
				model.packedVertices.Add(file.Read<real32>());
			}

			model.indices.Allocate(file.Read<uint32>(), MemoryType::PERMANENT);
			for (uint32 i = 0; i < model.indices.GetCapcity(); i++)
			{
				model.indices.Add(file.Read<uint32>());
			}

			SOLTRACE(String("Loaded model resource: ").Add(model.name).GetCStr());
			models.Put(model.id.number, model);
		}

		SOLINFO("Model loading complete");
	}

	void ResourceSystem::LoadAllTextures()
	{
		TextureResource invalid = {};
		invalid.name = "NONE/INVALID";
		textures.Put(0, invalid);

		BinaryAssetFile file = {};
		file.file = Platform::LoadEntireFile(String(PACKED_ASSET_PATH).Add("textures.bin"), false);

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

			textures.Put(texture.id.number, texture);
		}

		SOLINFO("Texture loading complete");
	}

	void ResourceSystem::LoadAllShaderPrograms()
	{
		ProgramResource invalid = {};
		invalid.name = "NONE/INVALID";
		programs.Put(0, invalid);

		BinaryAssetFile file = {};
		file.file = Platform::LoadEntireFile(String(PACKED_ASSET_PATH).Add("programs.bin"), false);

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
			programs.Put(shader.id.number, shader);
		}

		SOLINFO("Shader loading complete");
	}
}