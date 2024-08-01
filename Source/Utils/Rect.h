#pragma once

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
};

