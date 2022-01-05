#pragma once
#include "../SolarDefines.h"
#include "../core/SolarString.h"
#include "../core/SolarMath.h"

namespace sol
{
	enum class SOL_API ShaderStage
	{
		VERTEX,
		PIXEL,
		COMPUTE,
	};

	class SOL_API TextureFormat
	{
	public:
		enum class Value : uint8
		{
			INVALID = 0,
			R8G8B8A8_UNORM,
			R16G16_UNORM,
			R8_BYTE,
			R32_FLOAT,
			D32_FLOAT,
			R32_TYPELESS,
			R16_UNORM,
			D16_UNORM,
			R16_TYPELESS,
			R32G32_FLOAT,
			R32G32B32_FLOAT,
			R32G32B32A32_FLOAT,
			R16G16B16A16_FLOAT,
			COUNT,
		};

		TextureFormat()
		{
			value = Value::INVALID;
		}

		TextureFormat(Value v)
		{
			this->value = v;
		}

		inline String ToString() const
		{
			String copy = __STRINGS__[(uint32)value];

			return copy;
		}

		inline Value Get() const { return value; }

		inline static TextureFormat ValueOf(const uint32& v)
		{
			Assert(v < (uint32)Value::COUNT, "Invalid model id");
			return (TextureFormat::Value)v;
		}

		inline static TextureFormat ValueOf(const String& str)
		{
			uint32 count = (uint32)Value::COUNT;
			for (uint32 i = 0; i < count; i++)
			{
				if (str == __STRINGS__[i])
				{
					return ValueOf(i);
				}
			}

			return Value::INVALID;
		}

		inline bool operator==(const TextureFormat& rhs) const
		{
			return this->value == rhs.value;
		}

		inline bool operator!=(const TextureFormat& rhs) const
		{
			return this->value != rhs.value;
		}

	private:
		Value value;

		inline static const String __STRINGS__[] = {
			"INVALID",
			"R8G8B8A8_UNORM",
			"R16G16_UNORM",
			"R8_BYTE",
			"R32_FLOAT",
			"D32_FLOAT",
			"R32_TYPELESS",
			"R16_UNORM",
			"D16_UNORM",
			"R16_TYPELESS",
			"R32G32_FLOAT",
			"R32G32B32_FLOAT",
			"R32G32B32A32_FLOAT",
			"R16G16B16A16_FLOAT",
			"COUNT",
		};
	};

	class SOL_API ProgramStagesLayout
	{
	public:
		enum class Value : uint8
		{
			INVALID = 0,
			VERTEX_PIXEL,
			VERTEX_GEOMETRY_PIXEL,
			COMPUTE,
			COUNT
		};

		ProgramStagesLayout()
		{
			value = Value::INVALID;
		}

		ProgramStagesLayout(Value v)
		{
			this->value = v;
		}

		inline String ToString() const
		{
			String copy = __STRINGS__[(uint32)value];

			return copy;
		}

		inline Value Get() const { return value; }

		inline static ProgramStagesLayout ValueOf(const uint32& v)
		{
			Assert(v < (uint32)Value::COUNT, "Invalid model id");
			return (ProgramStagesLayout::Value)v;
		}

		inline static ProgramStagesLayout ValueOf(const String& str)
		{
			uint32 count = (uint32)Value::COUNT;
			for (uint32 i = 0; i < count; i++)
			{
				if (str == __STRINGS__[i])
				{
					return ValueOf(i);
				}
			}

			return Value::INVALID;
		}

		inline bool operator==(const ProgramStagesLayout& rhs) const
		{
			return this->value == rhs.value;
		}

		inline bool operator!=(const ProgramStagesLayout& rhs) const
		{
			return this->value != rhs.value;
		}

	private:
		Value value;

		inline static const String __STRINGS__[] = {
			"INVALID",
			"VERTEX_PIXEL",
			"VERTEX_GEOMETRY_PIXEL",
			"COMPUTE",
			"COUNT"
		};
	};

	class SOL_API ResourceCPUFlags
	{
	public:
		enum class Value : uint8
		{
			NONE = 0,
			READ,
			WRITE,
			READ_WRITE,
			COUNT
		};

		ResourceCPUFlags()
		{
			value = Value::NONE;
		}

		ResourceCPUFlags(Value v)
		{
			this->value = v;
		}

		inline String ToString() const
		{
			String copy = __STRINGS__[(uint32)value];

			return copy;
		}

		inline Value Get() const { return value; }

		inline static ResourceCPUFlags ValueOf(const uint32& v)
		{
			Assert(v < (uint32)Value::COUNT, "Invalid model id");
			return (ResourceCPUFlags::Value)v;
		}

		inline static ResourceCPUFlags ValueOf(const String& str)
		{
			uint32 count = (uint32)Value::COUNT;
			for (uint32 i = 0; i < count; i++)
			{
				if (str == __STRINGS__[i])
				{
					return ValueOf(i);
				}
			}

			return Value::NONE;
		}

		inline bool operator==(const ResourceCPUFlags& rhs) const
		{
			return this->value == rhs.value;
		}

		inline bool operator!=(const ResourceCPUFlags& rhs) const
		{
			return this->value != rhs.value;
		}

	private:
		Value value;

		inline static const String __STRINGS__[] = {
			"NONE",
			"READ",
			"WRITE",
			"READ_WRITE",
			"COUNT"
		};
	};

	class SOL_API TextureWrapMode
	{
	public:
		enum class Value : uint8
		{
			INVALID = 0,
			REPEAT,
			CLAMP_EDGE,
			COUNT
		};

		TextureWrapMode()
		{
			value = Value::INVALID;
		}

		TextureWrapMode(Value v)
		{
			this->value = v;
		}

		inline String ToString() const
		{
			String copy = __STRINGS__[(uint32)value];

			return copy;
		}

		inline Value Get() const { return value; }

		inline static TextureWrapMode ValueOf(const uint32& v)
		{
			Assert(v < (uint32)Value::COUNT, "Invalid model id");
			return (TextureWrapMode::Value)v;
		}

		inline static TextureWrapMode ValueOf(const String& str)
		{
			uint32 count = (uint32)Value::COUNT;
			for (uint32 i = 0; i < count; i++)
			{
				if (str == __STRINGS__[i])
				{
					return ValueOf(i);
				}
			}

			return Value::INVALID;
		}

		inline bool operator==(const TextureWrapMode& rhs) const
		{
			return this->value == rhs.value;
		}

		inline bool operator!=(const TextureWrapMode& rhs) const
		{
			return this->value != rhs.value;
		}

	private:
		Value value;

		inline static const String __STRINGS__[] = {
			"INVALID",
			"REPEAT",
			"CLAMP_EDGE",
			"COUNT"
		};
	};

	class SOL_API TextureFilterMode
	{
	public:
		enum class Value : uint8
		{
			INVALID = 0,
			POINT,
			BILINEAR,
			TRILINEAR,
			COUNT
		};

		TextureFilterMode()
		{
			value = Value::INVALID;
		}

		TextureFilterMode(Value v)
		{
			this->value = v;
		}

		inline String ToString() const
		{
			String copy = __STRINGS__[(uint32)value];

			return copy;
		}

		inline Value Get() const { return value; }

		inline static TextureFilterMode ValueOf(const uint32& v)
		{
			Assert(v < (uint32)Value::COUNT, "Invalid model id");
			return (TextureFilterMode::Value)v;
		}

		inline static TextureFilterMode ValueOf(const String& str)
		{
			uint32 count = (uint32)Value::COUNT;
			for (uint32 i = 0; i < count; i++)
			{
				if (str == __STRINGS__[i])
				{
					return ValueOf(i);
				}
			}

			return Value::INVALID;
		}

		inline bool operator==(const TextureFilterMode& rhs) const
		{
			return this->value == rhs.value;
		}

		inline bool operator!=(const TextureFilterMode& rhs) const
		{
			return this->value != rhs.value;
		}

	private:
		Value value;

		inline static const String __STRINGS__[] = {
			"INVALID",
			"POINT",
			"BILINEAR",
			"TRILINEAR",
			"COUNT"
		};
	};

	class SOL_API BindUsage
	{
	public:
		enum class Value : uint8
		{
			NONE = 0,
			SHADER_RESOURCE,
			RENDER_TARGET,
			DEPTH_SCENCIL_BUFFER,
			COMPUTER_SHADER_RESOURCE,
			COUNT,
			//DEPTH_STENCIL = D3D11_BIND_DEPTH_STENCIL
		};

		BindUsage()
		{
			value = Value::NONE;
		}

		BindUsage(Value v)
		{
			this->value = v;
		}

		inline String ToString() const
		{
			String copy = __STRINGS__[(uint32)value];

			return copy;
		}

		inline Value Get() const { return value; }

		inline static BindUsage ValueOf(const uint32& v)
		{
			Assert(v < (uint32)Value::COUNT, "Invalid model id");
			return (BindUsage::Value)v;
		}

		inline static BindUsage ValueOf(const String& str)
		{
			uint32 count = (uint32)Value::COUNT;
			for (uint32 i = 0; i < count; i++)
			{
				if (str == __STRINGS__[i])
				{
					return ValueOf(i);
				}
			}

			return Value::NONE;
		}

		inline bool operator==(const BindUsage& rhs) const
		{
			return this->value == rhs.value;
		}

		inline bool operator!=(const BindUsage& rhs) const
		{
			return this->value != rhs.value;
		}

	private:
		Value value;

		inline static const String __STRINGS__[] = {
			"NONE",
			"SHADER_RESOURCE",
			"RENDER_TARGET",
			"DEPTH_SCENCIL_BUFFER",
			"COMPUTER_SHADER_RESOURCE",
			"COUNT",
		};
	};

	class SOL_API VertexLayoutType
	{
	public:
		enum class Value : uint8
		{
			INVALID = 0,
			P,		// @NOTE: Postion
			P_PAD,	// @NOTE: Postion and a padd
			PNT,	// @NOTE: Postions, normal, texture coords(uv)
			PNTC,   // @NOTE: Postions, normal, texture coords(uv), vertex colour
			PNTM,	// @NOTE: Postions, normal, texture coords(uv), and an instanced model transform matrix
			TEXT,	// @NOTE: Layout for text rendering
			COUNT,
		};

		VertexLayoutType()
		{
			value = Value::INVALID;
		}

		VertexLayoutType(Value v)
		{
			this->value = v;
		}

		inline String ToString() const
		{
			String copy = __STRINGS__[(uint32)value];

			return copy;
		}

		inline uint32 GetStride() const
		{
			switch (value)
			{
			case Value::P: return 1;
			case Value::P_PAD: return 2;
			case Value::PNT: return 3 + 3 + 2;
			case Value::PNTC: return 3 + 3 + 2 + 4;
			case Value::PNTM: return 3 + 3 + 2 + 16;
			case Value::TEXT: return 4;
			}

			Assert(0, "INVALID STRIDE");
			return 0;
		}

		inline Value Get() const { return value; }

		inline static VertexLayoutType ValueOf(const uint32& v)
		{
			Assert(v < (uint32)Value::COUNT, "Invalid model id");
			return (VertexLayoutType::Value)v;
		}

		inline static VertexLayoutType ValueOf(const String& str)
		{
			uint32 count = (uint32)Value::COUNT;
			for (uint32 i = 0; i < count; i++)
			{
				if (str == __STRINGS__[i])
				{
					return ValueOf(i);
				}
			}

			return Value::INVALID;
		}

		inline bool operator==(const VertexLayoutType& rhs) const
		{
			return this->value == rhs.value;
		}

		inline bool operator!=(const VertexLayoutType& rhs) const
		{
			return this->value != rhs.value;
		}

	private:
		Value value;

		inline static const String __STRINGS__[] = {
			"INVALID",
			"P",
			"P_PAD",
			"PNT",
			"PNTC",
			"PNTM",
			"TEXT",
			"COUNT",
		};
	};

	//struct SOL_API FatVertex
	//{
	//	inline static constexpr uint32 MAX_BONE_INFLUENCE = 4;

	//	Vec3f position;
	//	Vec3f normal;
	//	Vec2f texCoords;
	//	Vec3f tangent;
	//	Vec3f bitangent;
	//	Vec4f colours;
	//	int32 boneIds[MAX_BONE_INFLUENCE] = {};
	//	real32 boneWeights[MAX_BONE_INFLUENCE] = {};
	//};

	struct SOL_API RenderPacket
	{

	};
}