#pragma once
#include "../Core.h"
#include "TextFile.h"
#include "BinaryFile.h"
#include "MetaProcessor.h"
#include "MaterialProcessor.h"


namespace sol
{
	class RoomProcessor
	{
	public:
		static bool8 CreateNewRoom(const String& path, const String& name);
		static bool8 SaveRoom(const String& path, Room* room);

		static bool8 SaveToTextFile(const String& path, RoomResource* room);
		static bool8 SaveToBinaryFile(const String& path, RoomResource* room);
		static bool8 ParseRoomTextFile(const String& path, RoomResource* room);
		static bool8 ParseRoomBinaryFile(const String& path, RoomResource* room);

		static std::vector<RoomResource> LoadAllRooms(const String& path);
	private:

	};


}