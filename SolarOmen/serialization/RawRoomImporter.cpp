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
				asset->player1StartPos = Vec3f(line.Split(':')[1]);
			}
			else if (line.StartsWith("Player2 Start Pos"))
			{
				asset->player2StartPos = Vec3f(line.Split(':')[1]);
			}
			else if (line.StartsWith("Entities"))
			{
				while (!line.StartsWith("Map") && line != "")
				{
					if (line.StartsWith("{"))
					{
						EntityAsset entityAsset = {};
						while (!line.StartsWith("}"))
						{
							line.RemoveWhiteSpace();
							if (line.StartsWith("Name"))
							{
								entityAsset.name = line.Split(':')[1];
							}
							else if (line.StartsWith("Tag"))
							{
								entityAsset.tag = Tag::ValueOf(line.Split(':')[1]);
							}
							else if (line.StartsWith("Transform"))
							{
								entityAsset.localTransform = Transform(line.SubStr(line.FindFirstOf(':') + 1));
							}
							else if (line.StartsWith("Render"))
							{
								ManagedArray<CString> values = line.Split(':');
								GetAssetState();

								entityAsset.renderComponent.enabled = true;
								if (values[1] != "NONE")
									entityAsset.renderComponent.modelId = GetAssetFromName(as->models.GetValueSet(), values[1]).id;
								if (values[2] != "NONE")
									entityAsset.renderComponent.textureId = GetAssetFromName(as->textures.GetValueSet(), values[2]).id;
								if (values[3] != "NONE")
									entityAsset.renderComponent.shaderId = GetAssetFromName(as->shaders.GetValueSet(), values[3]).id;
							}
							else if (line.StartsWith("Collider"))
							{
								ManagedArray<CString> values = line.Split(':');
								entityAsset.colliderComponent.enabled = true;
								entityAsset.colliderComponent.type = (ColliderType)values[1].ToInt32();

								switch (entityAsset.colliderComponent.type)
								{
								case ColliderType::SPHERE:
									entityAsset.colliderComponent.sphere = Sphere::Create(values[2]); break;

								case ColliderType::ALIGNED_BOUNDING_BOX:
									entityAsset.colliderComponent.alignedBox = AABB::Create(values[2], values[3]); break;

								default: Assert(0, "COLLIDER TYPE!!!");
								}
							}

							line = GetNextLine((const char*)file.data, (uint32)file.sizeBytes, &cursor);
						}

						asset->entities.Add(entityAsset);

						continue;
					}

					line = GetNextLine((const char*)file.data, (uint32)file.sizeBytes, &cursor);
				}
			}

		} while (line != "");

		return asset;
	}
}