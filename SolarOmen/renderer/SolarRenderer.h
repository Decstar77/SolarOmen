#pragma once
#include "../SolarAssets.h"
#include "../SolarOmen.h"
#include "../SolarSettings.h"
#include "../platform/SolarPlatform.h"
#include "dx11/SolarShader.h"

namespace cm
{
	inline DXGI_FORMAT GetTextureFormatToD3D(const TextureFormat& format)
	{
		switch (format)
		{
		case TextureFormat::R8G8B8A8_UNORM: return DXGI_FORMAT_R8G8B8A8_UNORM;
		case TextureFormat::R16G16_UNORM: return DXGI_FORMAT_R16G16_UNORM;
		case TextureFormat::R32_FLOAT: return DXGI_FORMAT_R32_FLOAT;
		case TextureFormat::D32_FLOAT: return DXGI_FORMAT_D32_FLOAT;
		case TextureFormat::R32_TYPELESS: return DXGI_FORMAT_R32_TYPELESS;
		case TextureFormat::R16_UNORM: return DXGI_FORMAT_R16_UNORM;
		case TextureFormat::D16_UNORM: return DXGI_FORMAT_D16_UNORM;
		case TextureFormat::R16_TYPELESS: return DXGI_FORMAT_R16_TYPELESS;
		case TextureFormat::R32G32_FLOAT: return DXGI_FORMAT_R32G32_FLOAT;
		case TextureFormat::R32G32B32_FLOAT: return DXGI_FORMAT_R32G32B32_FLOAT;
		case TextureFormat::R32G32B32A32_FLOAT: return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case TextureFormat::R16G16B16A16_FLOAT: return DXGI_FORMAT_R16G16B16A16_FLOAT;
		default: Assert(0, "TextureFormatToD3D ??");
		}

		return DXGI_FORMAT_UNKNOWN;
	}

	inline uint32 GetTextureFormatElementSizeBytes(const TextureFormat& format)
	{
		switch (format)
		{
		case TextureFormat::R8G8B8A8_UNORM: return sizeof(uint8);
		case TextureFormat::R16G16_UNORM: return sizeof(uint16);
		case TextureFormat::R32_FLOAT: return sizeof(real32);
		case TextureFormat::D32_FLOAT: return sizeof(real32);
		case TextureFormat::R32_TYPELESS: return sizeof(real32);
		case TextureFormat::R16_UNORM: return sizeof(uint16);
		case TextureFormat::D16_UNORM: return sizeof(uint16);
		case TextureFormat::R16_TYPELESS: return sizeof(uint16);
		case TextureFormat::R32G32_FLOAT: return sizeof(real32);
		case TextureFormat::R32G32B32_FLOAT: return sizeof(real32);
		case TextureFormat::R32G32B32A32_FLOAT: return sizeof(real32);
		case TextureFormat::R16G16B16A16_FLOAT: return sizeof(uint16);
		default: Assert(0, "TextureFormatToD3D ??");
		}

		return 0;
	}

	inline uint32 GetTextureFormatElementCount(const TextureFormat& format)
	{
		switch (format)
		{
		case TextureFormat::R8G8B8A8_UNORM: return 4;
		case TextureFormat::R16G16_UNORM: return 2;
		case TextureFormat::R32_FLOAT: return 1;
		case TextureFormat::D32_FLOAT: return 1;
		case TextureFormat::R32_TYPELESS: return 1;
		case TextureFormat::R16_UNORM: return 1;
		case TextureFormat::D16_UNORM: return 1;
		case TextureFormat::R16_TYPELESS: return 1;
		case TextureFormat::R32G32_FLOAT: return 2;
		case TextureFormat::R32G32B32_FLOAT: return 3;
		case TextureFormat::R32G32B32A32_FLOAT: return 4;
		case TextureFormat::R16G16B16A16_FLOAT: return 4;
		default: Assert(0, "TextureFormatToD3D ??");
		}

		return 0;
	}

	inline D3D11_TEXTURE_ADDRESS_MODE GetTextureWrapModeToD3D(const TextureWrapMode& wrap)
	{
		switch (wrap)
		{
		case TextureWrapMode::REPEAT: return D3D11_TEXTURE_ADDRESS_WRAP;
		case TextureWrapMode::CLAMP_EDGE:return D3D11_TEXTURE_ADDRESS_CLAMP;
		default: Assert(0, "TextureWrapModeToD3D ??");
		}

		return D3D11_TEXTURE_ADDRESS_WRAP;
	}

	inline D3D11_FILTER GetTextureFilterModeToD3D(const TextureFilterMode& mode)
	{
		switch (mode)
		{
		case TextureFilterMode::POINT:		return D3D11_FILTER_MIN_MAG_MIP_POINT;
		case TextureFilterMode::BILINEAR:	return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		case TextureFilterMode::TRILINEAR:	return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		default: Assert(0, "TextureFilterModeToD3D ??");
		}

		return D3D11_FILTER_MIN_MAG_MIP_POINT;
	}

	inline int32 GetTextureUsageToD3DBindFlags(const TextureUsage& usage)
	{
		switch (usage)
		{
		case TextureUsage::NONE:  return 0;
		case TextureUsage::SHADER_RESOURCE: return D3D11_BIND_SHADER_RESOURCE;
		case TextureUsage::RENDER_TARGET: return D3D11_BIND_RENDER_TARGET;
		case TextureUsage::DEPTH_SCENCIL_BUFFER: return D3D11_BIND_DEPTH_STENCIL;
		case TextureUsage::COMPUTER_SHADER_RESOURCE: return D3D11_BIND_UNORDERED_ACCESS;
		default: Assert(0, "TextureUsageToD3DBindFlags ??");
		}

		return 0;
	}

	inline int32 GetTextureCPUFlagsToD3DFlags(const TextureCPUFlags& flags)
	{
		switch (flags)
		{
		case TextureCPUFlags::NONE: return 0;
		case TextureCPUFlags::READ: return D3D11_CPU_ACCESS_READ;
		case TextureCPUFlags::WRITE: return D3D11_CPU_ACCESS_WRITE;
		case TextureCPUFlags::READ_WRITE: return D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
		}
		return 0;
	}



	struct RenderState;

	enum class TextureCubeFaceIndex {
		POSITIVE_X = 0,
		NEGATIVE_X = 1,
		POSITIVE_Y = 2,
		NEGATIVE_Y = 3,
		POSITIVE_Z = 4,
		NEGATIVE_Z = 5
	};

	struct MeshInstance
	{
		ModelId id;
		uint32 strideBytes;						// @NOTE: Used for rendering
		uint32 indexCount;						// @NOTE: Used for rendering
		ID3D11Buffer* vertexBuffer;
		ID3D11Buffer* indexBuffer;

		void Bind(RenderState* rs);
		void DrawIndexed(RenderState* rs);
	};

	struct TextureInstance
	{
		TextureId id;

		int32 currentRegister;
		ShaderStage shaderStage;

		int32 width;
		int32 height;
		TextureFormat format;
		TextureUsage usage[4];
		TextureCPUFlags cpuFlags;

		ID3D11Texture2D* texture;

		ID3D11ShaderResourceView* shaderView;
		ID3D11UnorderedAccessView* uavView;
		ID3D11DepthStencilView* depthView;
		ID3D11RenderTargetView* renderView;

		// @NOTE: A bit of hack to ensure that the IsValid call is true. 
		//		: Because we don't have all the info needed to be truely "valid"
		bool32 guaranteeValid;

		inline bool32 IsValid()
		{
			return (id != TextureId::Value::INVALID && width > 0 && height > 0 && texture) || (guaranteeValid);
		}

		void Bind(RenderState* rs, ShaderStage shaderStage, int32 register_);
		void Unbind(RenderState* rs);
	};

	struct TextureArrayInstance
	{
		TextureId id;

		int32 currentRegister;
		ShaderStage shaderStage;

		int32 width;
		int32 height;
		TextureFormat format;
		TextureUsage usage[4];

		ID3D11Texture2D* texture;

		int32 count;
		ID3D11ShaderResourceView* shaderView;
		ID3D11DepthStencilView* depthViews[8];

		inline bool32 IsValid()
		{
			return (id != TextureId::Value::INVALID && width > 0 && height > 0 && texture);
		}

		void Bind(RenderState* rs, ShaderStage shaderStage, int32 register_);
		void Unbind(RenderState* rs);
	};

	struct SamplerInstance
	{
		int32 currentRegister;
		TextureFilterMode filter;
		TextureWrapMode wrap;

		ID3D11SamplerState* sampler;

		void Bind(RenderState* rs, int32 register_);

		static SamplerInstance Create(RenderState* rs, TextureFilterMode filter, TextureWrapMode wrap, int32 currentRegister);
		static SamplerInstance CreateShadowPFC(RenderState* rs, int32 currentRegister);
	};

	struct CubeMapInstance
	{
		SkyboxId id;

		ID3D11Texture2D* texture = nullptr;
		ID3D11ShaderResourceView* shaderView = nullptr;

		ID3D11RenderTargetView* renderFaces[6];


		void Bind(RenderState* rs, ShaderStage shaderStage, int32 register_);
		void Unbind(RenderState* rs);
	};

	struct RenderTarget
	{
		union
		{
			struct
			{
				TextureInstance colourTarget0;
				TextureInstance colourTarget1;
				TextureInstance colourTarget2;
				TextureInstance colourTarget3;
			};

			TextureInstance colourTargets[4];
		};

		TextureInstance depthTarget;

		void Clear(RenderState* rs, const Vec4f& colour);
		void Bind(RenderState* rs);
		void Unbind(RenderState* rs);
	};

	struct RenderDebug
	{
		uint64 next;
		struct IDXGIInfoQueue* info_queue;

		ShaderInstance shader;
		ID3D11Buffer* vertex_buffer;
	};

	///struct EquirectangulartoCubemapPass
	///{
	///	ShaderInstance shader;
	///	TextureInstance colourBuffer;
	///	TextureInstance depthBuffer;
	///	RenderTarget renderTarget;
	///
	///	void Create(RenderState* rs);
	///	void Pass(RenderState* rs);
	///};

	struct RenderState
	{
		static inline RenderState* GlobalRenderState = nullptr;


		ID3D11Device* device;
		ID3D11DeviceContext* context;

		IDXGISwapChain* swapChain;
		RenderTarget swapChainRenderTarget;

		union
		{
			struct
			{
				ID3D11RasterizerState* rasterNormal;
				ID3D11RasterizerState* rasterNoFaceCullState;
				ID3D11RasterizerState* rasterFrontFaceCullingState;
			};
			ID3D11RasterizerState* rasterStates[8];
		};

		union
		{
			struct
			{
				// @NOTE: Enabled, WriteAll, Less
				ID3D11DepthStencilState* depthNormal;
				// @NOTE: Disabled, WriteAll, Always
				ID3D11DepthStencilState* depthOffState;
				// @NOTE: Enabled, WriteAll, Less Or Equal
				ID3D11DepthStencilState* depthLessEqualState;
			};
			ID3D11DepthStencilState* depthStates[8];
		};

		union
		{
			struct
			{
				// @NOTE:
				ID3D11BlendState* blendNormal;
			};
			ID3D11BlendState* blendStates[8];
		};

		union
		{
			struct
			{
				SamplerInstance pointRepeat;
				SamplerInstance bilinearRepeat;
				SamplerInstance trilinearRepeat;
				SamplerInstance shadowPFC;
			};
			SamplerInstance samplers[8];
		};

		ShaderInstance shaders[256];
		MeshInstance meshes[256];
		TextureInstance textures[256];

		//EquirectangulartoCubemapTechnique equiRecToCubeTech;
		TextureInstance eqiTexture;
		CubeMapInstance environmentMap;
		CubeMapInstance irradianceMap;

		uint32 reductionTargetCount;
		TextureInstance reductionStagingTex;
		TextureInstance reductionTargets[8];
		TextureArrayInstance shadowCascades;

		// @NOTE: Vertex shader const buffers
		ShaderConstBuffer vConstBuffers[16];
		// @NOTE: Pixel shader const buffers
		ShaderConstBuffer pConstBuffers[16];
		// @NOTE: Compute shader const buffers
		ShaderConstBuffer cConstBuffers[16];

		TextureInstance testTex;
		RenderDebug debug;
	};



	//static inline RenderState* renderState = nullptr;

	void LogDirectXDebugGetMessages(RenderDebug* debug);

	////////////////////////////////////////////////////
	// @NOTE: Platform layer call this
	////////////////////////////////////////////////////

	void InitializeRenderState(RenderState* rs, AssetState* as, PlatformState* ws);

	void RenderGame(RenderState* rs, AssetState* as, EntityRenderGroup* renderGroup, PlatformState* ws, Input* input);

	void PresentFrame(RenderState* rs, bool32 vsync);

	void ShutdownRenderState(RenderState* rs);
}