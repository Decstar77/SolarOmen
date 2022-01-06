#pragma once
#include "../SolarDefines.h"
#include "../core/SolarString.h"
#include "../core/SolarMath.h"
#include "../renderer/RendererTypes.h"

namespace sol
{
	struct ModelResource
	{
		String name;
		ResourceId id;
		VertexLayoutType layout;
		ManagedArray<real32> packedVertices;
		ManagedArray<uint32> indices;
	};

	struct TextureResource
	{
		String name;
		ResourceId id;
		int32 width;
		int32 height;
		TextureFormat format;
		ManagedArray<char> pixels;
		bool32 mips;
		BindUsage usage[4];
		ResourceCPUFlags cpuFlags;
	};

	struct ProgramResource
	{
		ResourceId id;
		String name;
		ProgramStagesLayout stageLayout;
		VertexLayoutType vertexLayout;
		ManagedArray<char> vertexData;
		ManagedArray<char> computeData;
		ManagedArray<char> pixelData;
	};

	class SOL_API Resources
	{
	public:
		static ManagedArray<ModelResource> GetAllModelResources();
		static ModelResource* GetModelResource(const ResourceId& id);
		static ModelResource* GetModelResource(const String& name);

		static ManagedArray<TextureResource> GetAllTextureResources();
		static TextureResource* GetTextureResource(const ResourceId& id);
		static TextureResource* GetTextureResource(const String& name);

		static ProgramResource* GetProgramResource(const ResourceId& id);
		static ProgramResource* GetProgramResource(const String& name);
	};
}