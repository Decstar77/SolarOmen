#pragma once
#include "SolarDefines.h"

namespace sol
{
	enum class EventCodeEngine
	{
		INVALID = 0,
		APPLICATION_QUIT = 1,
		KEY_PRESSED = 2,
		KEY_RELEASED = 3,
		BUTTON_PRESSED = 4,
		BUTTON_RELEASED = 5,
		MOUSE_MOVED = 6,
		MOUSE_WHEEL = 7,
		WINDOW_RESIZED = 8,
		DEBUG0 = 10,
		DEBUG1 = 11,
		DEBUG2 = 12,
		DEBUG3 = 13,
		DEBUG4 = 14,
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