#pragma once

// Bottom is bigger than the top
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
};

