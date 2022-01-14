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

		static ManagedArray<ProgramResource> GetAllProgramResources();
		static ProgramResource* GetProgramResource(const ResourceId& id);
		static ProgramResource* GetProgramResource(const String& name);
	};

	class SOL_API ModelGenerator
	{
	public:
		static ModelResource CreateQuad(real32 x, real32 y, real32 w, real32 h, real32 depth);
		static ModelResource CreateGrid(real32 width, real32 depth, uint32 m, uint32 n);
		static ModelResource CreateBox(real32 width, real32 height, real32 depth, uint32 numSubdivisions, VertexLayoutType layout);
		static ModelResource CreateSphere(real32 radius, uint32 sliceCount, uint32 stackCount);
		static ModelResource CreateGeosphere(real32 radius, uint32 numSubdivisions);
		static ModelResource CreateCylinder(real32 bottomRadius, real32 topRadius, real32 height, uint32 sliceCount, uint32 stackCount);
	};
}