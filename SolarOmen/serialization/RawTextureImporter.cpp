#include "RawTextureImporter.h"

#include "core/SolarCore.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../vendor/std_image/std_image.h"
namespace cm
{
	//int32 DEBUGCreateTexture(RenderState* rs, AssetState* as, CString path, bool flip, bool mips)
	//{
	//	PlatformFile file = DEBUGLoadEntireFile(path, false);

	//	int32 width = -1;
	//	int32 height = -1;
	//	int32 channels = -1;
	//	stbi_set_flip_vertically_on_load(flip);

	//	uint8* pixels = stbi_load_from_memory((stbi_uc*)file.data, (int32)file.size_bytes,
	//		&width, &height, &channels, 4);

	//	Assert(pixels, CString("Cannot find image: ").Add(path.GetCStr()).GetCStr());

	//	// @NOTE: We asked stbi to pad it with an aplha value wheather or not it is there.
	//	// @NOTE: The channel value is the TRUE amount of channel without pad/req comps count.
	//	// @NOTE: Thus we set it to 4
	//	channels = 4;

	//	TextureCreateInfo cinfo = {};
	//	cinfo.channels = channels;
	//	cinfo.width = width;
	//	cinfo.height = height;
	//	cinfo.pixels = pixels;
	//	cinfo.format = TextureFormat::R8G8B8A8_UNORM;
	//	cinfo.usage[0] = TextureUsage::SHADER_RESOURCE;

	//	TextureInstance texture = CreateTextureInstance2D(rs, cinfo);

	//	DEBUGFreeFile(&file);

	//	TextureData textureData = {};
	//	textureData.info = cinfo;
	//	textureData.name = Util::StripFilePathAndExtentions(path);

	//	int32 index = as->textureCount;
	//	rs->textures[index] = texture;
	//	as->texturesData[index] = {};
	//	as->texturesData[index] = textureData;
	//	as->textureCount++;

	//	return index;
	//}

	TextureAsset LoadTexture(CString filePath)
	{
		int32 width = -1;
		int32 height = -1;
		int32 channels = -1;

		if (Util::GetFileExtension(filePath) == "hdr")
		{
			stbi_set_flip_vertically_on_load(false);
			void* pixels = stbi_loadf(filePath.GetCStr(), &width, &height, &channels, 4);

			if (!pixels)
			{
				LOG(filePath.GetCStr() << stbi_failure_reason());
				Assert(0, "Could not load texture ");
			}

			// @NOTE: We asked stbi to pad it with an aplha value wheather or not it is there.
			// @NOTE: The channel value is the TRUE amount of channel without pad/req comps count.
			// @NOTE: Thus we set it to 4

			TextureAsset texture = {};
			texture.width = width;
			texture.height = height;
			texture.pixels = pixels;
			texture.format = TextureFormat::R32G32B32A32_FLOAT;
			texture.usage[0] = TextureUsage::SHADER_RESOURCE;

			return texture;
		}
		else
		{
			stbi_set_flip_vertically_on_load(false);
			void* pixels = stbi_load(filePath.GetCStr(), &width, &height, &channels, 4);

			if (!pixels)
			{
				LOG(filePath.GetCStr() << stbi_failure_reason());
				Assert(0, "Could not load texture ");
			}

			// @NOTE: We asked stbi to pad it with an aplha value wheather or not it is there.
			// @NOTE: The channel value is the TRUE amount of channel without pad/req comps count.
			// @NOTE: Thus we set it to 4

			TextureAsset texture = {};
			texture.width = width;
			texture.height = height;
			texture.pixels = pixels;
			texture.format = TextureFormat::R8G8B8A8_UNORM;
			texture.usage[0] = TextureUsage::SHADER_RESOURCE;
			// TODO: WE NEED TO CALL THIS 
			//stbi_image_free(data);

			return texture;
		}
	}

}
