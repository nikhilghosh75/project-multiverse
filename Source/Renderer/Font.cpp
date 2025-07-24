#include "Font.h"

#include "TTFRasterizer.h"

#include "ErrorManager.h"
#include "FileUtils.h"
#include "MathUtils.h"

#include <array>
#include <fstream>
#include <filesystem>
#include <iostream>

Font::Font(const std::string& filepath, FontLoadingOptions options)
	: defaultSize(options.pixelHeight), isInitialized(false), texture(nullptr)
{
	// Reads only from TTF file
	if (File::HasExtension(filepath, "ttf"))
	{
		ErrorManager::Get()->ReportError(ErrorSeverity::Error, "Font::Font", "Renderer", 0, "Font File is not TTF File");
	}

	std::filesystem::path path(filepath);
	std::size_t size = std::filesystem::file_size(path);

	std::ifstream file(filepath, std::ios::binary | std::ios::ate);
	file.seekg(0, std::ios::beg);

	std::vector<char> buffer(size);
	if (file.read(buffer.data(), size))
	{
		TTFRasterizer::ReadFromTTFBuffer(buffer.data(), buffer.size(), *this, options);
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
	return texture->GetTextureWidth();
}

float Font::GetBitmapHeight() const
{
	return texture->GetTextureHeight();
}

