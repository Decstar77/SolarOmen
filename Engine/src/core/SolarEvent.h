#pragma once
#include "../SolarDefines.h"

namespace sol
{
	enum class SOL_API EventCodeEngine
	{
		INVALID = 0,
		APPLICATION_QUIT,
		KEY_PRESSED,
		KEY_RELEASED,
		BUTTON_PRESSED,
		BUTTON_RELEASED5,
		MOUSE_MOVED,
		MOUSE_WHEEL,
		WINDOW_RESIZED,
		WINDOW_PUMP_MESSAGES,
		ON_RENDER_INITIALIZE,
		ON_RENDER_BEGIN,
		ON_RENDER_END,
		ON_RENDERER_SHUTDOWN,
		MAX_EVENT_CODE = 0xFF
	};

	struct SOL_API EventWindowResize
	{
		uint32 width;
		uint32 height;
	};

	struct SOL_API EventContext
	{
		char data[128];
	};

	SOL_API typedef bool8(*OnEventCallback)(uint16 eventCode, void* sender, void* listener, EventContext data);

	bool8 EventSystemInitialize();
	void EventSystemShutdown();

	class SOL_API EventSystem
	{
	public:
		static bool8 Register(uint16 eventCode, void* listener, OnEventCallback eventCallback);
		static bool8 Unregister(uint16 eventCode, void* listener, OnEventCallback eventCallback);

		template<typename T>
		inline static bool8 Fire(uint16 eventCode, void* sender, const T& t)
		{
			STATIC_ASSERT(sizeof(T) < sizeof(EventContext));
			EventContext context = {};
			context = *(EventContext*)&t;
			return Fire(eventCode, sender, context);
		}

		static bool8 Fire(uint16 eventCode, void* sender, EventContext context);
	};


}