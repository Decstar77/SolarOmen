#include "RawRoomImporter.h"

namespace cm
{
	CString GetNextLine(const char* data, uint32 length, uint32* cursor)
	{
		CString result = "";
		for (; *cursor < length; (*cursor)++)
		{
			if (data[*cursor] == '\n' || data[*cursor] == '\0')
			{
				(*cursor)++;
				break;
			}
			result.Add(data[*cursor]);
		}

		return result;
	}


	RoomAsset* LoadRoom(const CString& path)
	{
		RoomAsset* asset = GameMemory::PushTransientStruct<RoomAsset>();
		asset->name = Util::StripFilePathAndExtentions(path);

		PlatformFile file = Platform::LoadEntireFile(path, false);

		Assert(file.data, "LoadRoom");

		uint32 cursor = 0;
		CString line = "";

		do
		{
			line = GetNextLine((const char*)file.data, (uint32)file.sizeBytes, &cursor);

			if (line.StartsWith("Player1 Start Pos"))
			{
				asset->player1StartPos = StringToVec3<real32>(line.Split(':')[1]);
			}
			else if (line.StartsWith("Player2 Start Pos"))
			{
				asset->player2StartPos = StringToVec3<real32>(line.Split(':')[1]);
			}
			else if (line.StartsWith("Entities"))
			{

			}
			else if (line.StartsWith("Map"))
			{
				line = GetNextLine((const char*)file.data, (uint32)file.sizeBytes, &cursor);
				while (line != "")
				{
					ManagedArray<CString> values = line.Split(' ');
					for (uint32 i = 0; i < values.count; i++)
					{
						asset->map.Add(values[i].ToInt32());
					}
					line = GetNextLine((const char*)file.data, (uint32)file.sizeBytes, &cursor);
				}
			}

		} while (line != "");

		return asset;
	}
}