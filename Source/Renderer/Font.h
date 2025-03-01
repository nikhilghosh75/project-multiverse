#pragma once
#include "Rect.h"
#include "Texture.h"

#include "glm/vec2.hpp"

#include <vector>

struct FontLoadingOptions
{
	int pixelHeight;
};

class Font
{
public:
	Font(const std::string& filepath, FontLoadingOptions options);

	class Character
	{
	public:
		unsigned int charCode; // Is an unsigned int instead of a char for future support for unicode
		float xAdvance; // In Pixels
		Rect uvCoordinates; 
		glm::vec2 offset; // In Pixels

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
};