#include "SolarResources.h"
#include "platform/SolarPlatform.h"
#include "core/SolarLogging.h"

namespace sol
{
	static HashMap<ProgramResource> programs = {};

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

	ProgramResource* Resources::GetProgramResource(const String& name)
	{
		return GetResourcesFromName<ProgramResource>(programs.GetValueSet(), name);
	}

	static String PACKED_ASSET_PATH = "F:/codes/SolarOmen/SolarOmen-2/Assets/Packed/";
	bool8 ResourceSystem::Initialize()
	{
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

	void ResourceSystem::LoadAllShaderPrograms()
	{
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