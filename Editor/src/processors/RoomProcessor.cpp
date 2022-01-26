#include "RoomProcessor.h"
#include "FileProcessor.h"

namespace sol
{
	bool8 RoomProcessor::CreateNewRoom(const String& path, const String& name)
	{
		FileProcessor fileProcessor = {};
		auto roomFiles = fileProcessor.GetFilePaths(path, "txt");
		for (const auto& roomFile : roomFiles)
		{
			if (Util::StripFilePathAndExtentions(roomFile) == name)
			{
				SOLERROR("Another room has the same name !!");
				return false;
			}
		}

		RoomResource res = {};
		res.id = GenerateAssetId();
		res.name = name;

		return SaveToTextFile(path, &res);
	}

	bool8 RoomProcessor::SaveRoom(const String& path, Room* room)
	{
		SOLINFO(String("Saving...").Add(room->name).GetCStr());

		RoomResource res = {};
		room->ContructResource(&res);

		if (!res.id.IsValid()) {
			res.id = GenerateAssetId();
		}

		if (res.name.GetLength() == 0) {
			SOLERROR("Room doesn't have name");
			return false;
		}

		return RoomProcessor::SaveToTextFile(path, &res);
	}

	bool8 RoomProcessor::SaveToTextFile(const String& path, RoomResource* room)
	{
		TextFileWriter file = {};

		file.WriteLine(String("Version=").Add(1));
		file.WriteLine(String("Id=").Add(room->id.number));
		file.WriteLine(String("SkyboxId=").Add(room->skyBoxId));

		String fullPath = path;
		fullPath.Add(room->name).Add(".txt");
		return file.SaveToDisk(fullPath);
	}

	bool8 RoomProcessor::ParseRoomTextFile(const String& path, RoomResource* room)
	{
		TextFileReader file = {};

		if (file.Read(path))
		{
			room->name = Util::StripFilePathAndExtentions(path);

			for (String line = file.NextLine(); line != file.END_OF_FILE_STRING; line = file.NextLine())
			{
				if (line.StartsWith("Version")) {
					line.SubStr(line.FindFirstOf('=') + 1).ToInt32();
				}
				else if (line.StartsWith("Id")) {
					room->id.number = line.SubStr(line.FindFirstOf('=') + 1).ToUint64();
				}
				else if (line.StartsWith("SkyboxId")) {
					room->skyBoxId.number = line.SubStr(line.FindFirstOf('=') + 1).ToUint64();
				}
			}


			return true;
		}

		return false;
	}
}
