#pragma once
#include "../SolarDefines.h"
#include "../core/SolarString.h"
#include "../core/SolarMath.h"
#include "../renderer/RendererTypes.h"

namespace sol
{
	struct MeshTextures
	{
		ResourceId abledoTexture;
		ResourceId occlusionRoughnessMetallicTexture;
		ResourceId normalTexture;
		ResourceId emssiveTexture;
	};

	struct MeshResource
	{
		String name;
		VertexLayoutType layout;
		ManagedArray<real32> packedVertices;
		ManagedArray<uint32> indices;
	};

	struct ModelResource
	{
		ResourceId id;
		String name;
		ManagedArray<MeshResource> meshes;
		ManagedArray<MeshTextures> textures;
	};

	struct TextureResource
	{
		ResourceId id;
		String name;
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

	struct RoomResource
	{
		ResourceId id;
		String name;
		ResourceId skyBoxId;
	};

	class SOL_API Resources
	{
	public:
		static bool8 LoadAllModelResources();
		static ManagedArray<ModelResource> GetAllModelResources();
		static ModelResource* GetModelResource(const ResourceId& id);
		static ModelResource* GetModelResource(const String& name);

		static bool8 LoadAllTextureResources();
		static ManagedArray<TextureResource> GetAllTextureResources();
		static TextureResource* GetTextureResource(const ResourceId& id);
		static TextureResource* GetTextureResource(const String& name);

		static bool8 LoadAllProgramResources();
		static ManagedArray<ProgramResource> GetAllProgramResources();
		static ProgramResource* GetProgramResource(const ResourceId& id);
		static ProgramResource* GetProgramResource(const String& name);

		static bool8 LoadAllRoomResources();
		static ManagedArray<RoomResource> GetAllRoomResources();
		static RoomResource* GetRoomResource(const ResourceId& id);
		static RoomResource* GetRoomResource(const String& name);

	private:
		static bool8 Initialize();
		static void Shutdown();

		friend class Application;
	};

	class SOL_API ModelGenerator
	{
	public:
		static MeshResource CreateQuad(real32 x, real32 y, real32 w, real32 h, real32 depth);
		static MeshResource CreateGrid(real32 width, real32 depth, uint32 m, uint32 n);
		static MeshResource CreateBox(real32 width, real32 height, real32 depth, uint32 numSubdivisions, VertexLayoutType layout);
		static MeshResource CreateSphere(real32 radius, uint32 sliceCount, uint32 stackCount, VertexLayoutType layout);
		static MeshResource CreateGeosphere(real32 radius, uint32 numSubdivisions, VertexLayoutType layout);
		static MeshResource CreateCylinder(real32 bottomRadius, real32 topRadius, real32 height, uint32 sliceCount, uint32 stackCount, VertexLayoutType layout);
	};
}