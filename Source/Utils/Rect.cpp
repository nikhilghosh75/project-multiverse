#include "Rect.h"

#include "AssertUtils.h"

Rect::Rect()
	: top(0), bottom(0), left(0), right(0)
{
}

Rect::Rect(float _top, float _bottom, float _left, float _right)
	: top(_top), bottom(_bottom), left(_left), right(_right)
{
	ASSERT(bottom > top);
	ASSERT(right > left);
}

float Rect::Width() const
{
	return right - left;
}

float Rect::Height() const
{
	return  bottom - top;
}

void Rect::ResizeFromCenter(float newWidth, float newHeight)
{
	float centerX = (right + left) / 2;
	float centerY = (top + bottom) / 2;

	left = centerX - newWidth / 2;
	right = centerX + newWidth / 2;
	top = centerY - newHeight / 2;
	bottom = centerY + newHeight / 2;
}

bool Rect::IsPointInside(float x, float y)
{
	return left < x && right > x && bottom > y && top < y;
}
