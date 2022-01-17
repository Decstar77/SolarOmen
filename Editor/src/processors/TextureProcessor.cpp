#include "TextureProcessor.h"

namespace sol
{
	TextureProcessor::TextureProcessor()
	{

	}

	std::vector<Texture> TextureProcessor::LoadTextures(const std::vector<String>& texturePaths, const MetaProcessor& metaProcessor)
	{
		std::vector<Texture> textures;
		std::vector<String> textureNames;
		for (int32 i = 0; i < texturePaths.size(); i++) { textureNames.push_back(Util::StripFilePathAndExtentions(texturePaths.at(i))); }


		for (int32 i = 0; i < texturePaths.size(); i++)
		{
			String texturePath = texturePaths.at(i);
			String metaPath = metaProcessor.Find(textureNames.at(i));

			if (metaPath.GetLength() != 0)
			{
				TextureMetaFile metaFile = metaProcessor.ParseTextureMetaFile(metaPath);

				textures.emplace_back(texturePath, metaFile.id, Util::StripFilePathAndExtentions(texturePath));
			}
			else
			{
				LOG("No meta file for " << texturePath.GetCStr() << " something went very wrong !!");
			}
		}

		return textures;
	}

#define STB_IMAGE_IMPLEMENTATION
#include "../../../vendor/std_image/std_image.h"

	void Texture::LoadTexture(const String& path)
	{
		int32 width = -1;
		int32 height = -1;
		int32 channels = -1;

		stbi_set_flip_vertically_on_load(false);
		uint8* pixels = stbi_load(path.GetCStr(), &width, &height, &channels, 4);

		if (pixels)
		{
			// @NOTE: We asked stbi to pad it with an aplha value wheather or not it is there.
			// @NOTE: The channel value is the TRUE amount of channel without pad/req comps count.
			// @NOTE: Thus we set it to 4
			channels = 4;
			int32 size = width * height * channels;
			this->pixels.resize(size);
			for (int32 i = 0; i < this->pixels.size(); i++)
			{
				this->pixels.at(i) = pixels[i];
			}

			this->width = width;
			this->height = height;
			this->format = TextureFormat::Value::R8G8B8A8_UNORM;
			this->usage[0] = BindUsage::Value::SHADER_RESOURCE;

			stbi_image_free(pixels);

		}
		else
		{
			LOG(path.GetCStr() << stbi_failure_reason());
			Assert(0, "Could not load texture ");
		}
	}

	bool8 Texture::SaveBinaryData(BinaryFile* file) const
	{
		file->Write(id);
		file->Write(name);

		file->Write(mips);
		file->Write((uint32)width);
		file->Write((uint32)height);

		file->Write((uint8)format.Get());

		file->Write((uint8)usage[0].Get());
		file->Write((uint8)usage[1].Get());
		file->Write((uint8)usage[2].Get());
		file->Write((uint8)usage[3].Get());

		file->Write((uint8)cpuFlags.Get());

		file->Write(pixels);

		return true;
	}

}