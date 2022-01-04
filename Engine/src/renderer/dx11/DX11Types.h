#pragma once
#include "SolarDefines.h"
#include <d3d11.h>
#include <dxgi.h>
#include <dxgidebug.h>
#include <d3dcompiler.h>

#define DirectXDebugMessageCount 10

#define DXCHECK(call)                                                                                                   \
    {                                                                                                                   \
        dc.debug.next = dc.debug.info_queue->GetNumStoredMessages(DXGI_DEBUG_ALL);                    \
        HRESULT dxresult = (call);                                                                                      \
        if (FAILED(dxresult))                                                                                           \
        {                                                                                                               \
            char *output = nullptr;                                                                                     \
            FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER, \
                           NULL, dxresult, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&output, 0, NULL);         \
            if (output)                                                                                                 \
            {                                                                                                           \
                LogDirectXDebugGetMessages(&dc.debug);															\
            }                                                                                                           \
        }                                                                                                               \
    }

#define DXINFO(call)                                         \
    {                                                        \
        dc.debug.next = dc.debug.info_queue->GetNumStoredMessages(DXGI_DEBUG_ALL); \
		(call);                                              \
        LogDirectXDebugGetMessages(&dc.debug);		 \
    }


//@NOTE: This is not a debug thing, it will be used in release !!!
#define DXRELEASE(object)  \
    if ((object))          \
    {                      \
        object->Release(); \
        object = nullptr;  \
    }

namespace sol
{
	struct SwapChain
	{
		IDXGISwapChain* swapChain;
		ID3D11Texture2D* depthTexture;
		ID3D11DepthStencilView* depthView;
		ID3D11ShaderResourceView* depthShaderView;
		ID3D11RenderTargetView* renderView;
	};

	struct RenderDebug
	{
		uint64 next;
		struct IDXGIInfoQueue* info_queue;
		//ShaderInstance shader;
		//ID3D11Buffer* vertex_buffer;
	};

	void LogDirectXDebugGetMessages(RenderDebug* debug);

	struct DeviceContext
	{
		ID3D11Device* device;
		ID3D11DeviceContext* context;
#if SOL_DEBUG_RENDERING
		RenderDebug debug;
#endif
	};

	DeviceContext GetDeviceContext();

	// @TODO: Strings 
	class Topology
	{
	public:
		enum class Value
		{
			TRIANGLE_LIST,
			LINE_LIST,
		};

		Topology(Value value)
		{
			this->value = value;
		}

		inline D3D_PRIMITIVE_TOPOLOGY GetDXFormat() const
		{
			switch (value)
			{
			case Value::TRIANGLE_LIST: return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			case Value::LINE_LIST: return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
			}

			return {};
		}

	private:
		Value value;
	};

	struct RenderState
	{
		DeviceContext deviceContext;
		SwapChain swapChain;


		ID3D11RasterizerState* rasterBackFaceCullingState;
		ID3D11RasterizerState* rasterFrontFaceCullingState;
		ID3D11RasterizerState* rasterNoFaceCullState;

		ID3D11DepthStencilState* depthLessState;
		ID3D11DepthStencilState* depthOffState;
		ID3D11DepthStencilState* depthLessEqualState;

		ID3D11BlendState* blendNormal;
	};

}