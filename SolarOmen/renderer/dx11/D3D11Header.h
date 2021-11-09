#pragma once

#include <d3d11.h>
#include <dxgi.h>
#include <dxgidebug.h>
#include <d3dcompiler.h>

#define DirectXDebugMessageCount 10

#define DXCHECK(call)                                                                                                   \
    {                                                                                                                   \
        rs->debug.next = rs->debug.info_queue->GetNumStoredMessages(DXGI_DEBUG_ALL);                                     \
        HRESULT dxresult = (call);                                                                                      \
        if (FAILED(dxresult))                                                                                           \
        {                                                                                                               \
            char *output = nullptr;                                                                                     \
            FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER, \
                           NULL, dxresult, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&output, 0, NULL);         \
            if (output)                                                                                                 \
            {                                                                                                           \
                LogDirectXDebugGetMessages(&rs->debug);																	\
            }                                                                                                           \
        }                                                                                                               \
    }

#define DXINFO(call)                                         \
    {                                                        \
        rs->debug.next = rs->debug.info_queue->GetNumStoredMessages(DXGI_DEBUG_ALL); \
		(call);                                              \
        LogDirectXDebugGetMessages(&rs->debug);				 \
    }


//@NOTE: This is not a debug thing, it will be used in release !!!
#define DXRELEASE(object)  \
    if ((object))          \
    {                      \
        object->Release(); \
        object = nullptr;  \
    }


namespace cm
{
	class DeviceAndContext
	{
		ID3D11Device* device;
		ID3D11DeviceContext* context;
	};
}
