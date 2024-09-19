#include "Font.h"
#include <fstream>
#include <filesystem>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#define FONT_MIN_RESOLUTION 32
#define FONT_MAX_RESOLUTION 65536
#define FONT_MIN_RESOLUTION_PER_CHARACTER 32

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
	stbtt_fontinfo fontInfo;
	stbtt_InitFont(&fontInfo, reinterpret_cast<unsigned char*>(data), 0);

	// TODO: Maybe change this later
	defaultSize = 36.f;

	characters.reserve(fontInfo.numGlyphs);

	int _bitmapResolution = GetIdealBitmapResolution(fontInfo.numGlyphs);
	float scale = stbtt_ScaleForPixelHeight(&fontInfo, defaultSize);
	unsigned char* bitmap = (unsigned char*)malloc(_bitmapResolution * _bitmapResolution * sizeof(unsigned char));
	stbtt_bakedchar* bakedChars = new stbtt_bakedchar[fontInfo.numGlyphs];
	int result = stbtt_BakeFontBitmap(reinterpret_cast<unsigned char*>(data), 0, 60, bitmap, _bitmapResolution, _bitmapResolution, 32, fontInfo.numGlyphs, bakedChars);

	bitmapResolution = static_cast<float>(_bitmapResolution);
	texture = new Texture(bitmap, _bitmapResolution, _bitmapResolution, 1);
	int currentChar = 1;

	for (int i = 33; i < 256; i++)
	{
		int glyphIndex = stbtt_FindGlyphIndex(&fontInfo, i);
		if (glyphIndex == 0)
			continue;

		stbtt_bakedchar& bakedChar = bakedChars[currentChar];

		Font::Character charInfo;
		charInfo.charCode = i;

		if ((char)charInfo.charCode == 'o')
		{
			charInfo.charCode = i;
		}

		charInfo.xAdvance = bakedChar.xadvance;

		charInfo.uvCoordinates.left = (float)bakedChar.x0 / bitmapResolution;
		charInfo.uvCoordinates.bottom = (float)bakedChar.y0 / bitmapResolution;
		charInfo.uvCoordinates.right = (float)bakedChar.x1 / bitmapResolution;
		charInfo.uvCoordinates.top = (float)bakedChar.y1 / bitmapResolution;
		charInfo.offset = glm::vec2((float)bakedChar.xoff, (float)bakedChar.yoff);

		int x0, x1; // Unused currently
		stbtt_GetCodepointBitmapBox(&fontInfo, glyphIndex, scale, scale, &x0, &charInfo.ascent, &x1, &charInfo.decent);

		characters.push_back(charInfo);

		currentChar++;
	}

	delete[] bakedChars;
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
