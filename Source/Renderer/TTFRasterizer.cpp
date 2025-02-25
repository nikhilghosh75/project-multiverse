#include "TTFRasterizer.h"

#include "BezierUtils.h"
#include "ErrorManager.h"
#include "MathUtils.h"

#define FONT_MIN_RESOLUTION 32
#define FONT_MAX_RESOLUTION 65536
#define FONT_MIN_RESOLUTION_PER_CHARACTER 64

void TTFRasterizer::ReadFromTTFBuffer(void* data, uint32_t size, Font& font, FontLoadingOptions options)
{
	BinaryBufferStream bs(data, size, FileFlags::BigEndian);

	std::unordered_map<std::string, size_t> offsetTables = ReadTableLocation(bs);

	// Head table
	bs.GoTo(offsetTables["head"]);
	bs.Skip(18);

	int16_t xMin, yMin, xMax, yMax;
	int16_t unitsPerEm;
	int16_t indexToLocFormat;

	bs >> unitsPerEm;
	bs.Skip(16);
	bs >> xMin >> yMin >> xMax >> yMax;
	bs.Skip(6);
	bs >> indexToLocFormat;

	int numOfBytesPerLookup = indexToLocFormat == 0 ? 2 : 4;

	// Maxp table
	bs.GoTo(offsetTables["maxp"]);
	bs.Skip(4);

	uint16_t glyphCount;
	bs >> glyphCount;

	bs.GoTo(offsetTables["loca"]);
	std::vector<uint32_t> glyphLocations = GetAllGlyphLocations(bs, glyphCount, numOfBytesPerLookup, offsetTables["glyf"]);

	std::vector<GlyphMap> mapping = GetUnicodeToGlyphIndexMappings(bs, offsetTables["cmap"]);
	std::vector<Glyph> glyphs = ReadAllGlyphs(bs, glyphLocations, mapping);
	ApplyLayoutInfo(bs, glyphs, offsetTables["hhea"], offsetTables["hmtx"]);

	int textureDimensions = GetIdealBitmapResolution(mapping.size());
	std::vector<unsigned char> textureData(textureDimensions * textureDimensions, 0);
	PopulateTextureData(textureData, glyphs, font.characters, options.pixelHeight, yMin, yMax, textureDimensions);

	font.texture = new Texture(textureData.data(), textureDimensions, textureDimensions, 1);
}

std::unordered_map<std::string, size_t> TTFRasterizer::ReadTableLocation(BinaryBufferStream& bs)
{
	std::unordered_map<std::string, size_t> table;

	uint16_t numTables;

	bs.Skip(4); // scalarType
	bs >> numTables;
	bs.Skip(6);

	for (uint16_t i = 0; i < numTables; i++)
	{
		std::string tag = "    ";
		uint32_t checksum, offset, length;

		bs.Read(tag.data(), 4);
		bs >> checksum >> offset >> length;

		table.insert({ std::string(tag.data()), offset });
	}

	return table;
}

int TTFRasterizer::GetIdealBitmapResolution(int numCharacters)
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

std::vector<uint32_t> TTFRasterizer::GetAllGlyphLocations(BinaryBufferStream& fs, uint32_t glyphCount, int numOfBytesPerLookup, uint32_t glyphTableLocation)
{
	std::vector<uint32_t> glyphLocations;
	glyphLocations.reserve(glyphCount);

	for (uint32_t i = 0; i < glyphCount; i++)
	{
		if (numOfBytesPerLookup == 2)
		{
			uint16_t glyphLocation;
			fs >> glyphLocation;
			glyphLocations.push_back(static_cast<uint32_t>(glyphLocation) * 2 + glyphTableLocation);
		}
		else
		{
			uint32_t glyphLocation;
			fs >> glyphLocation;
			glyphLocations.push_back(glyphLocation + glyphTableLocation);
		}
	}

	return glyphLocations;
}

std::vector<TTFRasterizer::GlyphMap> TTFRasterizer::GetUnicodeToGlyphIndexMappings(BinaryBufferStream& fs, uint32_t cmapOffset)
{
	std::vector<TTFRasterizer::GlyphMap> map;

	fs.GoTo(cmapOffset);

	uint16_t version, subtablesCount;
	fs >> version >> subtablesCount;

	uint16_t subtableOffset = 0;
	uint16_t unicodeVersionID = 0;

	for (int i = 0; i < subtablesCount; i++)
	{
		uint16_t platformID, platformSpecificID;
		uint32_t offset;
		fs >> platformID >> platformSpecificID >> offset;

		if (platformID == 0)
		{
			if (platformSpecificID == 0 || platformSpecificID == 1 || platformSpecificID == 3 || platformSpecificID == 4)
			{
				subtableOffset = offset;
				unicodeVersionID = platformSpecificID;
			}
		}
		else if (platformID == 3)
		{
			if (platformSpecificID == 1 || platformSpecificID == 10)
			{
				subtableOffset = offset;
			}
		}
	}

	fs.GoTo(cmapOffset + subtableOffset);

	uint16_t format;
	fs >> format;

	// Format 12
	if (format == 12)
	{
		uint32_t numGroups;
		fs.Skip(10);
		fs >> numGroups;

		for (uint32_t i = 0; i < numGroups; i++)
		{
			uint32_t startCharCode, endCharCode, startGlyphIndex;
			fs >> startCharCode >> endCharCode >> startGlyphIndex;

			uint32_t numChars = endCharCode - startCharCode + 1;

			for (uint32_t charCodeOffset = 0; charCodeOffset < numChars; charCodeOffset++)
			{
				uint32_t charCode = startCharCode + charCodeOffset;
				uint32_t glyphIndex = startGlyphIndex + charCodeOffset;

				map.push_back({ glyphIndex, charCode });
			}
		}
	}
	else
	{
		ErrorManager::Get()->ReportError(ErrorSeverity::Warning, "GetUnicodeToGlyphIndexMappings", "Font", format, "TTF Font has incorrect format");
	}

	return map;
}

std::vector<TTFRasterizer::Glyph> TTFRasterizer::ReadAllGlyphs(BinaryBufferStream& fs, std::vector<uint32_t>& glyphLocations, std::vector<GlyphMap>& mappings)
{
	std::vector<TTFRasterizer::Glyph> glyphs(glyphLocations.size());

	for (int i = 0; i < mappings.size(); i++)
	{
		GlyphMap mapping = mappings[i];

		Glyph glyphData = ReadGlyph(fs, glyphLocations, mapping.glyphIndex);
		glyphData.unicodeValue = mappings[i].unicodeCharacter;
		glyphs[i] = glyphData;
	}

	return glyphs;
}

TTFRasterizer::Glyph TTFRasterizer::ReadGlyph(BinaryBufferStream& fs, std::vector<uint32_t>& glyphLocations, uint32_t glyphIndex)
{
	fs.GoTo(glyphLocations[glyphIndex]);
	int16_t contourCount;
	fs >> contourCount;

	// Glyph is either simple or compound
	// * Simple: outline data is stored here directly
	// * Compound: two or more simple glyphs need to be looked up, transformed, and combined
	bool isSimpleGlyph = contourCount >= 0;

	if (isSimpleGlyph)
	{
		return ReadSimpleGlyph(fs, glyphLocations, glyphIndex);
	}
	else
	{
		return ReadCompoundGlyph(fs, glyphLocations, glyphIndex);
	}
}

TTFRasterizer::Glyph TTFRasterizer::ReadSimpleGlyph(BinaryBufferStream& fs, std::vector<uint32_t>& glyphLocations, uint32_t glyphIndex)
{
	// Flag masks
	const int OnCurve = 0;
	const int IsSingleByteX = 1;
	const int IsSingleByteY = 2;
	const int Repeat = 3;
	const int InstructionX = 4;
	const int InstructionY = 5;

	Glyph glyph;
	glyph.glyphIndex = glyphIndex;

	fs.GoTo(glyphLocations[glyphIndex]);

	int16_t contourCount;
	fs >> contourCount >> glyph.minX >> glyph.minY >> glyph.maxX >> glyph.maxY;

	uint32_t numPoints = 0;
	glyph.contourEndIndices.reserve(contourCount);

	for (int16_t i = 0; i < contourCount; i++)
	{
		uint16_t contourEndIndex;
		fs >> contourEndIndex;
		glyph.contourEndIndices.push_back(contourEndIndex);
		numPoints = Math::Max(numPoints, contourEndIndex + 1);
	}

	int16_t instructionsLength;
	fs >> instructionsLength;
	fs.Skip(instructionsLength);

	// Read Flags
	std::vector<uint8_t> allFlags(numPoints);

	for (uint32_t i = 0; i < numPoints; i++)
	{
		uint8_t glyphFlags;
		fs >> glyphFlags;

		allFlags[i] = glyphFlags;

		if (((glyphFlags >> Repeat) & 1) == 1)
		{
			uint8_t repeatCount;
			fs >> repeatCount;
			for (int j = 0; j < repeatCount; j++)
			{
				i++;
				if (i < allFlags.size())
				{
					// TODO: Investigate why this doesn't always work (on character 050C (index 341))
					allFlags[i] = glyphFlags;
				}
			}
		}
	}

	glyph.points.resize(numPoints);

	// Read X Coordinates
	int16_t coordinate = 0;
	for (int i = 0; i < numPoints; i++)
	{
		if (((allFlags[i] >> IsSingleByteX) & 1) == 1)
		{
			uint8_t coordinateOffset;
			fs >> coordinateOffset;

			bool positiveOffset = ((allFlags[i] >> InstructionX) & 1) == 1;
			coordinate += positiveOffset ? coordinateOffset : -1 * (int16_t)coordinateOffset;
		}
		else if (((allFlags[i] >> InstructionX) & 1) == 0)
		{
			int16_t coordinateOffset;
			fs >> coordinateOffset;
			coordinate += coordinateOffset;
		}

		glyph.points[i].x = coordinate;
		glyph.points[i].onCurve = ((allFlags[i] >> OnCurve) & 1) == 1;
	}

	// Read Y Coordinates
	coordinate = 0;
	for (int i = 0; i < numPoints; i++)
	{
		if (((allFlags[i] >> IsSingleByteY) & 1) == 1)
		{
			uint8_t coordinateOffset;
			fs >> coordinateOffset;

			bool positiveOffset = ((allFlags[i] >> InstructionY) & 1) == 1;
			coordinate += positiveOffset ? coordinateOffset : -1 * (int16_t)coordinateOffset;
		}
		else if (((allFlags[i] >> InstructionY) & 1) == 0)
		{
			int16_t coordinateOffset;
			fs >> coordinateOffset;
			coordinate += coordinateOffset;
		}

		glyph.points[i].y = coordinate;
		glyph.points[i].onCurve = (allFlags[i] >> OnCurve) && 1 == 1;
	}

	return glyph;
}

TTFRasterizer::Glyph TTFRasterizer::ReadCompoundGlyph(BinaryBufferStream& fs, std::vector<uint32_t>& glyphLocations, uint32_t glyphIndex)
{
	Glyph compoundGlyph;
	compoundGlyph.glyphIndex = glyphIndex;

	fs.GoTo(glyphLocations[glyphIndex]);
	fs.Skip(2);

	fs >> compoundGlyph.minX >> compoundGlyph.minY >> compoundGlyph.maxX >> compoundGlyph.maxY;

	bool hasMoreGlyphs = true;
	while (hasMoreGlyphs)
	{
		uint16_t flags, compoundGlyphIndex;
		fs >> flags >> compoundGlyphIndex;

		if (glyphLocations[compoundGlyphIndex] == glyphIndex)
		{
			hasMoreGlyphs = false;
			break;
		}

		bool twoByteArguments = (flags & 1) == 1;
		bool xyValueArguments = ((flags >> 1) & 1) == 1;
		bool isSingleScaleValue = ((flags >> 3) & 1) == 1;
		bool isXAndYScale = ((flags >> 6) & 1) == 1;
		bool is2x2Matrix = ((flags >> 7) & 1) == 1;

		int16_t arg1, arg2;
		if (twoByteArguments)
		{
			fs >> arg1 >> arg2;
		}
		else
		{
			char arg1Byte, arg2Byte;
			fs >> arg1Byte >> arg2Byte;
			arg1 = static_cast<int16_t>(arg1Byte);
			arg2 = static_cast<int16_t>(arg2Byte);
		}

		float offsetX, offsetY = 0.f;

		if (xyValueArguments)
		{
			offsetX = arg1;
			offsetY = arg2;
		}

		float iHatX = 1;
		float iHatY = 0;
		float jHatX = 0;
		float jHatY = 1;

		if (isSingleScaleValue)
		{
			Fixed2Dot14 scaleValue;
			fs >> scaleValue;
			iHatX = scaleValue.GetFloat();
			jHatY = scaleValue.GetFloat();
		}
		else if (isXAndYScale)
		{
			Fixed2Dot14 xValue, yValue;
			fs >> xValue >> yValue;
			iHatX = xValue.GetFloat();
			iHatY = yValue.GetFloat();
		}
		else if (is2x2Matrix)
		{
			Fixed2Dot14 x0Value, x1Value, y0Value, y1Value;
			fs >> x0Value >> y0Value >> x1Value >> y1Value;
			iHatX = x0Value.GetFloat();
			iHatY = y0Value.GetFloat();
			jHatX = x1Value.GetFloat();
			jHatY = y1Value.GetFloat();
		}

		uint32_t currentFontLocation = fs.GetCurrentOffset();
		Glyph simpleGlyph = ReadGlyph(fs, glyphLocations, compoundGlyphIndex);
		fs.GoTo(currentFontLocation);

		for (int i = 0; i < simpleGlyph.contourEndIndices.size(); i++)
		{
			compoundGlyph.contourEndIndices.push_back(simpleGlyph.contourEndIndices[i] + compoundGlyph.points.size());
		}

		for (int i = 0; i < simpleGlyph.points.size(); i++)
		{
			float transformedX = iHatX * simpleGlyph.points[i].x + jHatX * simpleGlyph.points[i].x + offsetX;
			float transformedY = iHatY * simpleGlyph.points[i].y + jHatX * simpleGlyph.points[i].y + offsetY;

			compoundGlyph.points.push_back({ (int)transformedX, (int)transformedY, simpleGlyph.points[i].onCurve });
		}

		hasMoreGlyphs = ((flags >> 5) & 1) == 1;
	}

	return compoundGlyph;
}

void TTFRasterizer::ApplyLayoutInfo(BinaryBufferStream& fs, std::vector<Glyph>& glyphs, uint32_t hheaOffset, uint32_t hmtxOffset)
{
	fs.GoTo(hheaOffset);

	std::vector<int> advances;
	std::vector<int> leftBearings;

	int16_t lineGap, maxAdvanceWidth, advanceWidthsCount;
	fs.Skip(8);
	fs >> lineGap >> maxAdvanceWidth;
	fs.Skip(22);
	fs >> advanceWidthsCount;

	fs.GoTo(hmtxOffset);

	int16_t lastAdvanceWidth = 0;
	for (int i = 0; i < advanceWidthsCount; i++)
	{
		int16_t advanceWidth, leftSideBearing;
		fs >> advanceWidth >> leftSideBearing;
		lastAdvanceWidth = advanceWidth;

		advances.push_back(advanceWidth);
		leftBearings.push_back(leftSideBearing);
	}

	int numRemaining = glyphs.size() - advanceWidthsCount;

	for (int i = 0; i < numRemaining; i++)
	{
		int16_t leftSideBearing;
		fs >> leftSideBearing;

		advances.push_back(lastAdvanceWidth);
		leftBearings.push_back(leftSideBearing);
	}

	for (int i = 0; i < glyphs.size(); i++)
	{
		glyphs[i].advanceWidth = advances[i];
		glyphs[i].leftSideBearing = leftBearings[i];
	}
}

void TTFRasterizer::PopulateTextureData(std::vector<unsigned char>& textureData, std::vector<Glyph>& glyphs, std::vector<Font::Character>& characters, int pixelHeight, int16_t yMin, int16_t yMax, int textureDimensions)
{
	float rasterToPixelScale = (float)pixelHeight / (yMax - yMin);

	float lineTop = 0.f;
	float glyphStartX = 0.f;

	characters.resize(glyphs.size());

	for (int i = 0; i < glyphs.size(); i++)
	{
		uint16_t translatedYMin = glyphs[i].minY - yMin;
		uint16_t translatedYMax = glyphs[i].maxY - yMin;

		// Set the character data up
		characters[i].uvCoordinates.top = (translatedYMin * rasterToPixelScale / textureDimensions) + lineTop;
		characters[i].uvCoordinates.bottom = (translatedYMax * rasterToPixelScale / textureDimensions) + lineTop;
		characters[i].uvCoordinates.left = glyphStartX;
		characters[i].uvCoordinates.right = ((glyphs[i].maxX - glyphs[i].minX) * rasterToPixelScale / textureDimensions) + glyphStartX;

		characters[i].charCode = glyphs[i].unicodeValue;
		characters[i].offset.x = glyphs[i].minX * rasterToPixelScale;
		characters[i].offset.y = glyphs[i].minY * rasterToPixelScale;
		characters[i].xAdvance = glyphs[i].advanceWidth * rasterToPixelScale;

		RenderGlyphToTexture(textureData, glyphs[i], characters[i].uvCoordinates, textureDimensions);

		glyphStartX += (characters[i].uvCoordinates.right - characters[i].uvCoordinates.left) + (5.f / textureDimensions);
		if (glyphStartX > 0.95f)
		{
			glyphStartX = 0;
			lineTop += ((float)pixelHeight + 1.f) / (float)textureDimensions;
		}
	}
}

void TTFRasterizer::RenderGlyphToTexture(std::vector<unsigned char>& textureData, Glyph& glyph, Rect& uvRect, int textureDimensions)
{
	Rect pixelRect = ConvertUVRectToPixel(uvRect, textureDimensions);
	int16_t glyphWidth = glyph.maxX - glyph.minX;
	int16_t glyphHeight = glyph.maxY - glyph.minY;

	if (glyph.glyphIndex == 13)
	{
		glyphWidth = glyph.maxX - glyph.minX;
	}

	for (int y = pixelRect.top; y <= pixelRect.bottom; y++)
	{
		for (int x = pixelRect.left; x <= pixelRect.right; x++)
		{
			int16_t glyphX = ((x - pixelRect.left) / pixelRect.Width()) * glyphWidth + glyph.minX;
			int16_t glyphY = ((pixelRect.Height() - (y - pixelRect.top)) / pixelRect.Height()) * glyphHeight + glyph.minY;

			float alpha = GetAlphaAtPoint(glyph, glyphX, glyphY);
			unsigned char alphaInt = alpha * 255;
			size_t index = y * textureDimensions + x;
			textureData[index] = alphaInt;
		}
	}
}

Rect TTFRasterizer::ConvertUVRectToPixel(Rect rect, int textureDimensions)
{
	Rect returnValue;

	returnValue.bottom = std::roundf(rect.bottom * textureDimensions);
	returnValue.top = std::roundf(rect.top * textureDimensions);
	returnValue.right = std::roundf(rect.right * textureDimensions);
	returnValue.left = std::roundf(rect.left * textureDimensions);

	return returnValue;
}

float TTFRasterizer::GetAlphaAtPoint(Glyph& glyph, int16_t x, int16_t y)
{
	int intersectionCount = 0;

	for (int i = 0; i < glyph.contourEndIndices.size(); i++)
	{
		int contourStart = i == 0 ? 0 : glyph.contourEndIndices[i - 1] + 1;
		int contourEnd = glyph.contourEndIndices[i];
		for (int j = contourStart; j <= contourEnd; j++)
		{
			Point& point1 = glyph.points[j];
			Point& point2 = glyph.points[j == contourEnd ? contourStart : j + 1];

			// If both points are on the curve, then it is a straight line
			if (point1.onCurve && point2.onCurve)
			{
				if (DoesLineIntersectHorizontalRay(point1, point2, { x, y, false }))
				{
					intersectionCount++;
				}
			}
			// If neither point is on the curve, then points 1 and 2 are control points of two different beziers
			// We only consider the bezier that point 1 is a part of in this calculation
			else if (!point1.onCurve && !point2.onCurve)
			{
				Point& point0 = j == contourStart ? glyph.points[contourEnd] : glyph.points[j - 1];
				Point midpoint = { (point1.x + point2.x) / 2, (point1.y + point2.y) / 2, false };

				float a = point0.y - 2 * point1.y + midpoint.y;
				float b = 2 * (point1.y - point0.y);
				float c = point0.y - y;

				std::pair<std::optional<float>, std::optional<float>> result = Math::QuadraticFormula(a, b, c);
				if (result.first && IsValidIntersection(point0, point1, point2, x, *result.first))
				{
					intersectionCount++;
				}
				if (result.second && IsValidIntersection(point0, point1, point2, x, *result.second))
				{
					intersectionCount++;
				}
			}
			// If the first point is on the curve but the second is off the curve, we look at the next point.
			// If the next point is on the curve, then this is a bezier. Otherwise, we'll handle it next iteration
			else if (point1.onCurve && !point2.onCurve)
			{
				int point3Index = j + 1 == contourEnd ? contourStart + 1 : (j + 2 == contourEnd ? contourStart : j + 2);
				Point& point3 = glyph.points[point3Index];

				if (point3.onCurve)
				{
					float a = point1.y - 2 * point2.y + point3.y;
					float b = 2 * (point2.y - point1.y);
					float c = point1.y - y;

					std::pair<std::optional<float>, std::optional<float>> result = Math::QuadraticFormula(a, b, c);
					if (result.first && IsValidIntersection(point1, point2, point3, x, *result.first))
					{
						intersectionCount++;
					}
					if (result.second && IsValidIntersection(point1, point2, point3, x, *result.second))
					{
						intersectionCount++;
					}
				}
			}
			// If the first point is off the curve, but the second is on the curve, we already handled it?
		}
	}

	return intersectionCount % 2 == 0 ? 0.0f : 1.0f;
}

bool TTFRasterizer::DoesLineIntersectHorizontalRay(Point a, Point b, Point rayOrigin)
{
	if ((a.y > rayOrigin.y && b.y > rayOrigin.y) || (a.y < rayOrigin.y && b.y < rayOrigin.y)) 
	{
		// The segment is completely above or below the ray
		return false;
	}

	if ((a.y > b.y && b.y == rayOrigin.y) || (a.y < b.y && a.y == rayOrigin.y))
	{
		// The ray intersects the bottom of the segment, we say only the top segment intersects
		return false;
	}

	if (a.y == b.y) 
	{
		// If it's horizontal, we don't intersect
		return false; 
	}

	// Find the x-coordinate of the intersection using linear interpolation
	float intersectX = a.x + (rayOrigin.y - a.y) * (b.x - a.x) / (b.y - a.y);

	// Check if the intersection is to the right of rayOrigin.x
	return intersectX >= rayOrigin.x;
}

bool TTFRasterizer::IsValidIntersection(Point a, Point b, Point c, float rayOriginX, float t)
{
	bool isOnCurveSegment = t >= 0 && t < 0.99f;
	bool isRightOfRay = Bezier::CalculateQuadratic({ a.x, a.y }, { b.x, b.y }, { c.x, c.y }, t).x >= rayOriginX;
	return isRightOfRay && isOnCurveSegment;
}
