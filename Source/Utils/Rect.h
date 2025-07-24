#pragma once

#include <vector>

/*
* A class representing a rectangle
* Because this code primarily works in rendering space, the bottom value is always bigger than the top
*/
class Rect
{
public:
	Rect();
	Rect(float _top, float _bottom, float _left, float _right);

	float top;
	float bottom;
	float left;
	float right;

	float Width() const;
	float Height() const;

	float CenterX() const;
	float CenterY() const;

	void ResizeFromCenter(float newWidth, float newHeight);

	bool IsPointInside(float x, float y);

	static Rect GetBoundingRect(const std::vector<Rect>& rects);
};

