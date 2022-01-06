#pragma once
#include "../SolarDefines.h"
#include "../core/SolarString.h"
#include "../core/SolarMath.h"
#include "../renderer/RendererTypes.h"

namespace sol
{
	struct SOL_API ModelResource
	{
		String name;
		ResourceId id;
		VertexLayoutType layout;
		ManagedArray<real32> packedVertices;
		ManagedArray<uint32> indices;
	};

	struct SOL_API TextureResource
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

	struct SOL_API ProgramResource
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
		static ModelResource* GetModelResource(const ResourceId& name);
		static ModelResource* GetModelResource(const String& name);

		static ManagedArray<TextureResource> GetAllTextureResources();

		static ProgramResource* GetProgramResource(const ResourceId& name);
		static ProgramResource* GetProgramResource(const String& name);
	};
}