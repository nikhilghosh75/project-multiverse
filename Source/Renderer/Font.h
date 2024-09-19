#pragma once
#include "glm/vec2.hpp"
#include "Rect.h"
#include "Texture.h"
#include <vector>

enum class CharacterSet : uint8_t
{
	ANSI = 0x00,
	DEFAULT = 0x01,
	SYMBOLSET = 0x02,
	UNICODESET = 0x03,
	MAC = 0x4D,
	SHIFTJIS = 0x80,
	HANGUL = 0x81,
	JOHAB = 0x82,
	GB2312 = 0x86,
	CHINESEBIGS = 0x88,
	GREEK = 0xA1,
	TURKISH = 0xA2,
	VIETNAMESE = 0xA3,
	HEBREW = 0xB1,
	ARABIC = 0xB2,
	BALTIC = 0xBA,
	RUSSIAN = 0xCC,
	THAI = 0xDE,
	EASTEUROPE = 0xEE,
	OEMCHARSET = 0xFF
};

class Font
{
public:
	Font();
	Font(const std::string& filepath);

	class Character
	{
	public:
		unsigned int charCode;
		float xAdvance;
		Rect uvCoordinates;
		glm::vec2 offset;
		int ascent;
		int decent;

		bool operator<(const Character& other) const
		{
			return this->charCode < other.charCode;
		}

		bool operator>(const Character& other) const
		{
			return this->charCode > other.charCode;
		}
	};

	std::vector<Character> characters;
	Texture* texture;

	int GetCharacterIndex(char c);
	float GetPixelScale(float fontSize);

	float GetBitmapWidth() const;
	float GetBitmapHeight() const;

private:
	float bitmapResolution;
	float defaultSize;
	bool isInitialized;
	CharacterSet characterSet;

	void ReadFromTTFBuffer(void* data, uint32_t size);

	int GetIdealBitmapResolution(int numCharacters);
};