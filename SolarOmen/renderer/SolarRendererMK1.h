#pragma once

#if 0
#include "../platform/SolarPlatform.h"
#include "../SolarOmen.h"
#include "../FileParsers.h"

#include "../SolarMemory.h"

#include <d3d11.h>
#include <dxgi.h>
#include <dxgidebug.h>
#include <d3dcompiler.h>

namespace cm
{

	enum class TextureCubeFaceIndex {
		POSITIVE_X = 0,
		NEGATIVE_X = 1,
		POSITIVE_Y = 2,
		NEGATIVE_Y = 3,
		POSITIVE_Z = 4,
		NEGATIVE_Z = 5
	};


	struct ComputeShaderInstance
	{
		ID3D11ComputeShader* cs_shader;
	};

	struct ShaderInstance
	{
		ID3D11VertexShader* vs_shader;
		ID3D11PixelShader* ps_shader;
		ID3D11InputLayout* layout;
	};

	struct ShaderBuffer
	{
		ID3D11Buffer* buffer = nullptr;
		int32 size_bytes;
		int32 copy_ptr;

		// @TODO: This may be really dump but oh well
		uint8 staging_buffer[2048];

		inline void CopyVec3fIntoShaderBuffer(const Vec3f& a)
		{
			Assert(copy_ptr + 4 <= size_bytes / 4, "DXConstBuffer::CopyInVec3f buffer overrun");

			real32* ptr = (real32*)staging_buffer;

			ptr[copy_ptr] = a.x;
			copy_ptr++;
			ptr[copy_ptr] = a.y;
			copy_ptr++;
			ptr[copy_ptr] = a.z;
			copy_ptr++;
			ptr[copy_ptr] = 0.0f;
			copy_ptr++;
		}

		inline void CopyVec4fIntoShaderBuffer(const Vec4f& a)
		{
			Assert(copy_ptr + 4 <= size_bytes / 4, "DXConstBuffer::CopyInVec3f buffer overrun");

			real32* ptr = (real32*)staging_buffer;

			ptr[copy_ptr] = a.x;
			copy_ptr++;
			ptr[copy_ptr] = a.y;
			copy_ptr++;
			ptr[copy_ptr] = a.z;
			copy_ptr++;
			ptr[copy_ptr] = a.w;
			copy_ptr++;
		}

		inline void CopyMat4fIntoShaderBuffer(const Mat4f& a, bool32 transpose)
		{
			Assert(copy_ptr + 16 <= size_bytes / 4, "DXConstBuffer::CopyInMat4f buffer overrun");

			real32* ptr = (real32*)staging_buffer;

			// @TODO: There is probably a faster way, without the call to transopse, but that is way to premature to optimze
			Mat4f aT = Transpose(a);
			for (int32 i = 0; i < 16; i++)
			{
				ptr[copy_ptr] = aT.ptr[i];
				copy_ptr++;
			}
		}
	};

	inline Triangle CreateTriangleFromMeshData(MeshData* mesh, int32 triangleIndex, const Mat4f& transformation)
	{
		int32 index = triangleIndex * 3;
		int32 vertex_index = mesh->indices[index] * mesh->packed_stride;

		Vec3f v1 = Vec3f(
			mesh->packed_vertices[vertex_index],
			mesh->packed_vertices[vertex_index + 1],
			mesh->packed_vertices[vertex_index + 2]);

		vertex_index = mesh->indices[index + 1] * mesh->packed_stride;

		Vec3f v2 = Vec3f(
			mesh->packed_vertices[vertex_index],
			mesh->packed_vertices[vertex_index + 1],
			mesh->packed_vertices[vertex_index + 2]);


		vertex_index = mesh->indices[index + 2] * mesh->packed_stride;

		Vec3f v3 = Vec3f(
			mesh->packed_vertices[vertex_index],
			mesh->packed_vertices[vertex_index + 1],
			mesh->packed_vertices[vertex_index + 2]);

		v1 = Vec3f(Vec4f(v1, 1.0f) * transformation);
		v2 = Vec3f(Vec4f(v2, 1.0f) * transformation);
		v3 = Vec3f(Vec4f(v3, 1.0f) * transformation);

		Triangle tri = CreateTriangle(v1, v2, v3);

		return tri;
	}

	struct MeshInstance
	{
		uint32 stride_bytes;	// @NOTE: Used for rendering
		uint32 index_count;		// @NOTE: Used for rendering
		ID3D11Buffer* vertex_buffer = nullptr;
		ID3D11Buffer* index_buffer = nullptr;
	};

	struct InstancedData
	{
		uint32 sizeBytes;
		uint32 strideBytes;	// @NOTE: Used for rendering
		ID3D11Buffer* buffer = nullptr;
	};

	struct SceneVertexBuffer
	{
		int32 boxCount;
		int32 boxCapcity;
		ID3D11Buffer* buffer;
		ID3D11ShaderResourceView* view;
	};




	struct TextureInstance
	{
		int32 current_resgister;
		// @TODO: Do we want the create info here ?
		TextureCreateInfo info;
		ID3D11Texture2D* texture = nullptr;

		ID3D11ShaderResourceView* view = nullptr;
		ID3D11UnorderedAccessView* uavView = nullptr;
	};


	struct SamplerCreateInfo
	{
		TextureFilterMode filter;
		TextureWrapMode wrap;

		// @TODO: Mip levels;
		//int32 mip_levels;
	};

	struct SamplerInstance
	{
		int32 current_resigter;
		SamplerCreateInfo info;
		ID3D11SamplerState* sampler;
	};

	struct CubeMapInstance
	{
		ID3D11Texture2D* cubemap = nullptr;
		ID3D11ShaderResourceView* view = nullptr;
		ID3D11SamplerState* sampler = nullptr;
	};

	struct RenderTargetCubeMap
	{
		ID3D11Texture2D* cubeMap;
		ID3D11Texture2D* depthMap;
		ID3D11ShaderResourceView* view;
		ID3D11ShaderResourceView* faceFaceViews[6];
		ID3D11DepthStencilView* depthViews[6];
		ID3D11RenderTargetView* faceViews[6];
	};

	struct RasterizerInstance
	{
		ID3D11RasterizerState* rasterizer;
	};

	struct RenderTarget
	{
		int32 width;
		int32 height;

		union
		{
			struct
			{
				TextureInstance colourTexture0;
				TextureInstance colourTexture1;
				TextureInstance colourTexture2;
				TextureInstance colourTexture3;
			};

			TextureInstance colourTextures[4];
		};

		union
		{
			struct
			{
				ID3D11RenderTargetView* renderTarget0;
				ID3D11RenderTargetView* renderTarget1;
				ID3D11RenderTargetView* renderTarget2;
				ID3D11RenderTargetView* renderTarget3;
			};

			ID3D11RenderTargetView* renderTargets[4];
		};

		ID3D11Texture2D* depth_texture;
		ID3D11DepthStencilView* depth_target;
	};

	struct RenderDebug
	{
		uint64 next;
		struct IDXGIInfoQueue* info_queue;

		ShaderInstance shader;
		ID3D11Buffer* vertex_buffer;
	};

	struct Swapchain
	{
		IDXGISwapChain* swapchain = nullptr;
		RenderTarget render_target;
	};

	struct Voxel
	{
		int32 xIndex;
		int32 yIndex;
		int32 zIndex;
		int32 boxIndex;

		inline bool IsValid() {
			return xIndex != -1 && yIndex != -1 && zIndex != -1;
		}
	};

	// @TODO: Move this !!
	struct Morton {

		static constexpr uint64 GridDim = 1024;
		static uint64 Encode(uint64 x, uint64 y, uint64 z) {
			return Split(x) | (Split(y) << 1) | (Split(z) << 2);
		}

	private:
		static constexpr int32 log_bits = 5;

		static uint64 Split(uint64 x) {
			uint64_t mask = (UINT64_C(1) << (1 << log_bits)) - 1;
			for (int i = log_bits, n = 1 << log_bits; i > 0; --i, n >>= 1) {
				mask = (mask | (mask << n)) & ~(mask << (n / 2));
				x = (x | (x << n)) & mask;
			}
			return x;
		}
	};

	// @TODO: Move this !!
	struct BVHNode
	{
		AABB box;
		int32 primCount;
		int32 firstIndex;

		inline bool IsLeaf() { return primCount != 0; }
	};

	// @TODO: Move this !!
	struct BVH
	{
		int32 maxPrimCount;

		int32 nodeCount;
		BVHNode nodes[40000];

		int32 primitiveCount;
		int32 primitiveIndices[20000];
	};

	struct PackedBox
	{
		Quatf orientation;
		Vec4f centerPad;
		Vec4f extents;
	};

	void BuildBVH(BVH* bvh, AABB* boxes, Vec3f* centers, int32 count);

	struct RenderState
	{
		ID3D11Device* device = nullptr;
		ID3D11DeviceContext* context = nullptr;

		ID3D11DepthStencilState* ds_state = nullptr;
		ID3D11DepthStencilState* depthOffState = nullptr;
		ID3D11DepthStencilState* depthLessEqualState = nullptr;
		ID3D11DepthStencilState* shadow_depth_state = nullptr;
		ID3D11RasterizerState* rs_standard_state = nullptr;
		ID3D11RasterizerState* noFaceCullState = nullptr;
		ID3D11RasterizerState* frontFaceCullingState = nullptr;

		ID3D11BlendState* blend_state;

		Swapchain swapchain;

		bool buildBVH;
		BVH bvh;

		int32 particleShader;
		int32 bloomFilterShader;
		int32 gaussianBlurShader;
		int32 unlit_shader;
		int32 post_processing_shader;
		int32 phong_shader;
		int32 defferdPhongShader;
		int32 shadow_shader;
		int32 gbufferShader;
		int32 debugShadowVolumeShader;
		int32 pbr_shader;

		ComputeShaderInstance shadowRayShader;
		ComputeShaderInstance testSphereGenShader;
		ComputeShaderInstance gaussianBlurShaderHorizontal;
		ComputeShaderInstance gaussianBlurShaderVertical;

		MeshInstance screen_space_quad;

		InstancedData particleTransformBuffer;
		SceneVertexBuffer sceneBuffer;

		RenderTarget gbufferRenderTarget;
		RenderTarget forwardPassRenderTarget;

		RenderTarget bloomRenderTarget;
		RenderTarget msaa_render_target;
		RenderTarget post_render_target;

		TextureInstance temporayTexture;

		RenderTargetCubeMap shadowCube;

		TextureInstance shadowBuffer;

		SamplerInstance linear_sampler;
		SamplerInstance point_sampler;
		SamplerInstance pcfSampler;

		int32 pbr_texture;

		TextureInstance textures[256];

		ShaderInstance shaders[256];

		int32 cube_mesh;
		int32 wall_mesh;
		int32 floor_mesh;
		int32 demo_room_mesh;

		MeshInstance meshes[512];

		ShaderBuffer vs_matrices;
		ShaderBuffer ps_lighting_info;
		ShaderBuffer ps_lighting_constants;
		ShaderBuffer psTransientData;

		CubeMapInstance skybox;
		ShaderInstance skybox_shader;
		MeshInstance skybox_mesh;

		int32 shadowAtlasSize;
		real32 pointLightNearPlane;
		real32 pointLightFarPlane;
		int32 pointLightShadowSize;
		real32 pointLightShadowAtlasDelta;
		real32 bloomThreshold;
		real32 bloomSoftness;

		RenderDebug debug;
	};

	inline static bool32 IsValidMesh(RenderState* rs, int32 index)
	{
		if (index < 0 || index >= ArrayCount(rs->meshes))
		{
			return 0;
		}

		MeshInstance* result = &rs->meshes[index];
		return result->vertex_buffer ? 1 : 0;
	}

	inline static MeshInstance* LookUpMeshInstance(RenderState* rs, int32 index)
	{
		Assert(IsValidMesh(rs, index), "LookUpMeshInstance, has invalid index");
		MeshInstance* result = &rs->meshes[index];
		Assert(result->vertex_buffer, "LookUpMeshInstance, mesh is invalid");

		return result;
	}

	inline bool32 IsValidShader(RenderState* rs, int32 index)
	{
		Assert(index >= 0 && index < ArrayCount(rs->shaders), "IsValidShader has invalid index");
		ShaderInstance* result = &rs->shaders[index];

		return result->vs_shader && result->ps_shader && result->layout ? 1 : 0;
	}

	inline ShaderInstance* LookUpShaderInstance(RenderState* rs, int32 index)
	{
		Assert(IsValidShader(rs, index), "Invalid shader lookup");
		ShaderInstance* shader = &rs->shaders[index];

		return shader;
	}

	inline bool32 IsValidTexture(RenderState* rs, int32 index)
	{
		Assert(index >= 0 && index < ArrayCount(rs->textures), "IsValidTexture has invalid index");
		TextureInstance* result = &rs->textures[index];

		return result->texture && result->view ? 1 : 0;
	}

	inline TextureInstance* LookUpTextureInstance(RenderState* rs, int32 index)
	{
		Assert(IsValidTexture(rs, index), "Invalid texture lookup");
		TextureInstance* texture = &rs->textures[index];

		return texture;
	}

	void DEBUGDrawBVH(RenderState* rs, BVH* bvh, bool32 leafOnly = false, int32 node = -1);

	void InitializeDirectX(HWND window, RenderState* rs, AssetState* as);

	void InitializeDirectXDebugDrawing(RenderState* rs, AssetState* as);

	void InitializeDirectXDebugLogging(RenderState* rs);

	void LogDirectXDebugGetMessages(RenderDebug* debug);

	RenderTargetCubeMap CreatePointLightCubeMap(RenderState* rs);

	void BindSampler(RenderState* rs, SamplerInstance* sampler, int32 register_);

	void BindTexture(RenderState* rs, TextureInstance* texture, ShaderStage stage, int32 register_);

	void BindTextureCompute(RenderState* rs, TextureInstance* texture, int32 register_);

	void BindCubeMap(RenderState* rs, CubeMapInstance* cubemap, int32 register_);

	void BindMesh(RenderState* rs, MeshInstance* mesh);

	void BindInstancedData(RenderState* rs, InstancedData* data);

	void BindShader(RenderState* rs, ShaderInstance* shader);

	void BindComputeShader(RenderState* rs, ComputeShaderInstance* shader);

	ShaderBuffer CreateShaderBuffer(RenderState* rs, int32 size_bytes);

	void CopyVec4iIntoShaderBuffer(ShaderBuffer* buffer, const Vec4i& a);

	void CopyVec3fIntoShaderBuffer(ShaderBuffer* buffer, const Vec3f& a);

	void CopyVec4fIntoShaderBuffer(ShaderBuffer* buffer, const Vec4f& a);

	void CopyMat4fIntoShaderBuffer(ShaderBuffer* buffer, const Mat4f& a, bool32 transpose = true);

	void UpdateShaderBuffer(RenderState* rs, ShaderBuffer* buffer);

	void BindShaderBuffer(RenderState* rs, ShaderBuffer* buffer, ShaderStage stage, int32 register_);

	SamplerInstance CreateSamplerInstance(RenderState* rs, const SamplerCreateInfo& cinfo);

	SamplerInstance CreateSamplerInstanceComapison(RenderState* rs, const SamplerCreateInfo& cinfo);

	TextureInstance CreateTextureInstance2D(RenderState* rs, const TextureCreateInfo& cinfo);

	CubeMapInstance CreateCubeMapInstance(RenderState* rs, uint8* sides[6], int32 width, int32 height, int32 channels);

	void UpdateInstanceData(RenderState* rs, InstancedData* instancedData, void* data, int32 sizeBytes);

	InstancedData CreateInstanceData(RenderState* rs, int32 sizeBytes, int32 strideBytes);

	SceneVertexBuffer CreateSceneBuffer(RenderState* rs, int32 boxCount);

	MeshInstance CreateMeshInstance(RenderState* rs, real32* vertices, int32 vertex_count, uint32* indices, int32 indices_count);

	void BindRenderTarget(RenderState* rs, RenderTarget* rt);

	void ClearRenderTarget(RenderState* rs, RenderTarget* rt, const Vec4f& colour);

	RenderTarget CreateRenderTarget(RenderState* rs, TextureInstance* colour_buffer, bool32 depth);

	RenderTarget CreateRenderTarget(RenderState* rs, int32 width, int32 height, TextureFormat format);

	void AddTextureToRenderTarget(RenderState* rs, RenderTarget* renderTarget, TextureInstance* texture);

	RenderTarget CreateMutliSampleRenderTarget(RenderState* rs, int32 width, int32 height, int32 samples);

	void FreeShader(ShaderInstance* shader);

	void RenderScreenSpaceQuad(RenderState* rs);

	ShaderInstance DEBUGCreateShaderFromSource(RenderState* rs, CString vertex_file, CString pixel_file);

	ComputeShaderInstance DEBUGCreateComputeShaderFromBinary(RenderState* rs, CString file);

	CubeMapInstance DEBUGCreateCubeMap(RenderState* rs, CString path);

	// @HACK: Remove this when writing final renderer, it's duplicated in file parsers
	int32 DEBUGCreateShaderFromBinary(RenderState* rs, AssetState* as, const VertexShaderLayout& layout, CString vertex_file, CString pixel_file);


	////////////////////////////////////////////////////
	// @NOTE: Platform layer call this
	////////////////////////////////////////////////////

	void InitializeRenderState(RenderState* rs, AssetState* as, PlatformState* ws, TransientState* ts);

	void RenderGame(RenderState* rs, AssetState* as, EntityRenderGroup* renderGroup, PlatformState* ws, Input* input);

	void PresentFrame(RenderState* rs, bool32 vsync);
}
#endif