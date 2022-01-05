#pragma once
#include "../SolarDefines.h"
#include "../core/SolarString.h"
#include "../core/SolarMath.h"
#include "../renderer/RendererTypes.h"

namespace sol
{
	struct SOL_API ResourceId
	{
		union
		{
			uint64 number;
			char chars[8];
			STATIC_ASSERT(sizeof(uint64) == sizeof(char[8]));
		};

		inline String ToString() const { return String(chars); }
		inline bool operator==(const ResourceId& rhs) const { return this->number == rhs.number; }
		inline bool operator!=(const ResourceId& rhs) const { return this->number != rhs.number; }
		inline bool IsValid() const { return number != 0; }
		inline operator uint64() const { return number; }
	};

	struct SOL_API ModelResource
	{
		String name;
		ResourceId id;
		VertexLayoutType layout;
		ManagedArray<real32> packedVertices;
		ManagedArray<uint32> indices;
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
		static ProgramResource* GetProgramResource(const ResourceId& name);
		static ProgramResource* GetProgramResource(const String& name);

		static ManagedArray<ModelResource> GetAllModelResources();
		static ModelResource* GetModelResource(const ResourceId& name);
		static ModelResource* GetModelResource(const String& name);
	};
}