#pragma once

#include "Font.h"

#include "BinaryBufferStream.h"

#include <string>
#include <unordered_map>
#include <vector>

class TTFRasterizer
{
public:
	static void ReadFromTTFBuffer(void* data, uint32_t size, Font& font, FontLoadingOptions options);

private:
	struct Point
	{
		int x;
		int y;
		bool onCurve;
	};

	struct Glyph
	{
		uint32_t unicodeValue;
		uint32_t glyphIndex;
		std::vector<Point> points;
		std::vector<int> contourEndIndices;
		int advanceWidth;
		int leftSideBearing;

		int16_t minX;
		int16_t maxX;
		int16_t minY;
		int16_t maxY;

		int16_t GetWidth() const { return maxX - minX; }
		int16_t GetHeight() const { return maxY - minY; }
	};

	struct GlyphMap
	{
		uint32_t glyphIndex;
		uint32_t unicodeCharacter;
	};

	static std::unordered_map<std::string, size_t> ReadTableLocations(BinaryBufferStream& bs);

	static int GetIdealBitmapResolution(int numCharacters);
	static std::vector<uint32_t> GetAllGlyphLocations(BinaryBufferStream& fs, uint32_t glyphCount, int numOfBytesPerLookup, uint32_t glyphTableLocation);

	static std::vector<GlyphMap> GetUnicodeToGlyphIndexMappings(BinaryBufferStream& fs, uint32_t cmapOffset);
	static std::vector<Glyph> ReadAllGlyphs(BinaryBufferStream& fs, std::vector<uint32_t>& glyphLocations, std::vector<GlyphMap>& mappings);

	static Glyph ReadGlyph(BinaryBufferStream& fs, std::vector<uint32_t>& glyphLocations, uint32_t glyphIndex);

	static Glyph ReadSimpleGlyph(BinaryBufferStream& fs, std::vector<uint32_t>& glyphLocations, uint32_t glyphIndex);
	static Glyph ReadCompoundGlyph(BinaryBufferStream& fs, std::vector<uint32_t>& glyphLocations, uint32_t glyphIndex);

	static void ApplyLayoutInfo(BinaryBufferStream& fs, std::vector<Glyph>& glyphs, uint32_t hheaOffset, uint32_t hmtxOffset);

	static void PopulateTextureData(std::vector<unsigned char>& textureData, std::vector<Glyph>& glyphs, std::vector<Font::Character>& characters, int pixelHeight, int16_t yMin, int16_t yMax, int textureDimensions);
	static void RenderGlyphToTexture(std::vector<unsigned char>& textureData, Glyph& glyph, Rect& uvRect, int textureDimensions);

	static Rect ConvertUVRectToPixel(Rect rect, int textureDimensions);

	static float GetAlphaAtPoint(Glyph& glyph, int16_t x, int16_t y);

	static bool DoesLineIntersectHorizontalRay(Point a, Point b, Point rayOrigin);
	static bool IsValidIntersection(Point a, Point b, Point c, float rayOriginX, float t);
};