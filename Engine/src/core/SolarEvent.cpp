#include "SolarEvent.h"
#include "SolarContainers.h"
#include "SolarLogging.h"

namespace sol
{
	struct EventEntry
	{
		void* listener;
		OnEventCallback callback;
	};

	static FixedArray<ManagedArray<EventEntry>, UINT16_MAX> events = {};

	bool8 EventSystemInitialize()
	{
		events.count = UINT16_MAX;
		SOLINFO("Event system started");
		return 1;
	}

	void EventSystemShutdown()
	{
		for (uint32 i = 0; i < events.count; i++)
		{
			events[i].Clear();
		}

		SOLINFO("Event system shutdown");
	}

	bool8 EventSystem::Register(uint16 eventCode, void* listener, OnEventCallback eventCallback)
	{
		ManagedArray<EventEntry>* entries = &events[eventCode];

		if (entries->GetCapcity() == 0)
		{
			entries->Allocate(64, MemoryType::PERMANENT);
		}

		for (uint32 i = 0; i < entries->count; i++)
		{
			if (entries->Get(i)->callback == listener)
			{
				SOLWARN("Registering listener that is already listening");
				return false;
			};
		}

		EventEntry entry = {};
		entry.callback = eventCallback;
		entry.listener = listener;

		entries->Add(entry);

		return true;
	}

	bool8 EventSystem::Unregister(uint16 eventCode, void* listener, OnEventCallback eventCallback)
	{
		ManagedArray<EventEntry>* entries = &events[eventCode];

		if (entries->GetCapcity() == 0)
		{
			return false;
		}

		for (uint32 i = 0; i < entries->count; i++)
		{
			if (entries->Get(i)->callback == listener)
			{
				entries->Remove(i);
				return true;
			};
		}

		return false;
	}

	SOL_API bool8 EventSystem::Fire(uint16 eventCode, void* sender, EventContext context)
	{
		ManagedArray<EventEntry>* entries = &events[eventCode];

		if (entries->GetCapcity() == 0)
		{
			return false;
		}

		for (uint32 i = 0; i < entries->count; i++)
		{
			EventEntry* entry = entries->Get(i);
			if (entry->callback(eventCode, sender, entry->listener, context))
			{
				return true;
			};
		}

		return false;
	}


}