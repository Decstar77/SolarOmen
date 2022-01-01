#include "RawFontImporter.h"

#include "vendor/freetype/include/ft2build.h"
#include FT_FREETYPE_H

namespace cm
{
	FontAsset LoadFont(const CString& path)
	{
		FontAsset asset = {};
		FT_Library ft;
		if (!FT_Init_FreeType(&ft))
		{
			FT_Face face;
			if (!FT_New_Face(ft, path.GetCStr(), 0, &face))
			{
				FT_Set_Pixel_Sizes(face, 0, 48);

				asset.chars.Allocate(128, MemoryType::PERMANENT);
				for (unsigned char c = 0; c < 128; c++)
				{
					if (!FT_Load_Char(face, c, FT_LOAD_RENDER))
					{
						FontCharacter fontChar = {};
						int32 width = face->glyph->bitmap.width;
						int32 height = face->glyph->bitmap.rows;
						int32 count = width * height;
						fontChar.character = c;
						fontChar.size = Vec2i(width, height);
						fontChar.bearing = Vec2i(face->glyph->bitmap_left, face->glyph->bitmap_top);
						fontChar.advance = (int32)face->glyph->advance.x;
						fontChar.data.Allocate(count, MemoryType::PERMANENT);

						for (int32 i = 0; i < count; i++)
						{
							fontChar.data[i] = face->glyph->bitmap.buffer[i];
						}

						asset.chars.Add(fontChar);
					}
				}
				FT_Done_Face(face);
			}
			FT_Done_FreeType(ft);
		}

		return asset;
	}
}