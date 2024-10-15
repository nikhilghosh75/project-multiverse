#include "Font.h"
#include <fstream>
#include <filesystem>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include <ft2build.h>
#include FT_FREETYPE_H 

#define FONT_MIN_RESOLUTION 32
#define FONT_MAX_RESOLUTION 65536
#define FONT_MIN_RESOLUTION_PER_CHARACTER 32

static FT_Library library;
static bool libraryInitialized = false;

Font::Font()
{
	isInitialized = false;
}

Font::Font(const std::string& filepath)
{
	std::filesystem::path path(filepath);
	std::size_t size = std::filesystem::file_size(path);

	std::ifstream file(filepath, std::ios::binary | std::ios::ate);
	file.seekg(0, std::ios::beg);

	std::vector<char> buffer(size);
	if (file.read(buffer.data(), size))
	{
		ReadFromTTFBuffer(buffer.data(), buffer.size());
		isInitialized = true;
	}
}

int Font::GetCharacterIndex(char c)
{
	unsigned int character = static_cast<unsigned int>(c);

	// TODO: Optimize to binary search
	for (int i = 0; i < characters.size(); i++)
	{
		if (characters[i].charCode == character)
		{
			return i;
		}
	}

	return -1;
}

float Font::GetPixelScale(float fontSize)
{
	return fontSize / defaultSize;
}

float Font::GetBitmapWidth() const
{
	return bitmapResolution;
}

float Font::GetBitmapHeight() const
{
	return bitmapResolution;
}

void Font::ReadFromTTFBuffer(void* data, uint32_t size)
{
	if (!libraryInitialized)
	{
		if (FT_Init_FreeType(&library))
		{
			// TODO: Handle Error
		}

		libraryInitialized = true;
	}

	FT_Face face;
	FT_New_Memory_Face(library, (FT_Byte*)data, size, 0, &face);
	// TODO: Handle face loading errors

	// TODO: Change later
	defaultSize = 36.f;
	FT_Set_Pixel_Sizes(face, 0, 36);

	// TODO: Change later
	bitmapResolution = GetIdealBitmapResolution(233);

	const int textureDimensions = 1024;
	std::vector<unsigned char> textureData(textureDimensions * textureDimensions, 0);

	int offsetX = 0, offsetY = 0;
	int maxRowHeight = 0;

	for (int i = 33; i < 256; i++)
	{
		if (FT_Load_Char(face, i, FT_LOAD_RENDER))
		{
			// TODO: Handle error
			continue;
		}

		FT_Bitmap& bitmap = face->glyph->bitmap;

		if (offsetX + bitmap.width >= textureDimensions)
		{
			offsetX = 0;
			offsetY += maxRowHeight;
			maxRowHeight = 0;
		}

		if (bitmap.rows > maxRowHeight) 
		{
			maxRowHeight = bitmap.rows;
		}

		for (int y = 0; y < bitmap.rows; ++y) 
		{
			for (int x = 0; x < bitmap.width; ++x) 
			{
				textureData[(offsetY + y) * textureDimensions + (offsetX + x)] = bitmap.buffer[y * bitmap.pitch + x];
			}
		}

		Font::Character charInfo;
		charInfo.charCode = i;
		charInfo.xAdvance = face->glyph->advance.x >> 6;
		charInfo.offset.x = face->glyph->bitmap_left;
		charInfo.offset.y = face->glyph->bitmap_top;

		charInfo.uvCoordinates.top = (float)offsetY / (float)textureDimensions;
		charInfo.uvCoordinates.left = (float)offsetX / (float)textureDimensions;
		charInfo.uvCoordinates.right = (float)(offsetX + bitmap.width) / (float)textureDimensions;
		charInfo.uvCoordinates.bottom = (float)(offsetY + bitmap.rows) / (float)textureDimensions;
		
		characters.push_back(charInfo);

		offsetX += bitmap.width + 1;
	}

	texture = new Texture(textureData.data(), textureDimensions, textureDimensions, 1);
}

int Font::GetIdealBitmapResolution(int numCharacters)
{
	int resolutionToCheck = FONT_MIN_RESOLUTION;
	while (resolutionToCheck < FONT_MAX_RESOLUTION)
	{
		int charResolution = sqrt(resolutionToCheck * resolutionToCheck / numCharacters);
		if (charResolution > FONT_MIN_RESOLUTION_PER_CHARACTER)
		{
			return resolutionToCheck;
		}
		resolutionToCheck *= 2;
	}

	return resolutionToCheck;
}
