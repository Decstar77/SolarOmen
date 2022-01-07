#pragma once
#include "../SolarDefines.h"
#include "SolarString.h"

namespace sol
{
	class SOL_API EngineEvent
	{
	public:
		enum class Value
		{
			INVALID = 0,
			APPLICATION_QUIT,
			KEY_PRESSED,
			KEY_RELEASED,
			BUTTON_PRESSED,
			BUTTON_RELEASED,
			MOUSE_MOVED,
			MOUSE_WHEEL,
			WINDOW_RESIZED,
			WINDOW_PUMP_MESSAGES,
			ON_RENDER_INITIALIZE,
			ON_RENDER_BEGIN,
			ON_RENDER_END,
			ON_RENDERER_SHUTDOWN,
			COUNT,
			MAX_EVENT_CODE = 0xFF
		};

		EngineEvent()
		{
			value = Value::INVALID;
		}

		EngineEvent(Value v)
		{
			this->value = v;
		}

		inline String ToString() const
		{
			String copy = __STRINGS__[(uint32)value];

			return copy;
		}

		inline Value Get() const { return value; }

		inline operator uint16() const { return (uint16)value; }

		inline static EngineEvent ValueOf(const uint32& v)
		{
			Assert(v < (uint32)Value::COUNT, "Invalid model id");
			return (EngineEvent::Value)v;
		}

		inline static EngineEvent ValueOf(const String& str)
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

		inline bool operator==(const EngineEvent& rhs) const { return this->value == rhs.value; }
		inline bool operator!=(const EngineEvent& rhs) const { return this->value != rhs.value; }

	private:
		Value value;

		inline static const char* __STRINGS__[] = {
			"INVALID"
			"APPLICATION_QUIT",
			"KEY_PRESSED",
			"KEY_RELEASED",
			"BUTTON_PRESSED",
			"BUTTON_RELEASED",
			"MOUSE_MOVED",
			"MOUSE_WHEEL",
			"WINDOW_RESIZED",
			"WINDOW_PUMP_MESSAGES",
			"ON_RENDER_INITIALIZE",
			"ON_RENDER_BEGIN",
			"ON_RENDER_END",
			"ON_RENDERER_SHUTDOWN",
			"COUNT",
			"MAX_EVENT_CODE"
		};
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