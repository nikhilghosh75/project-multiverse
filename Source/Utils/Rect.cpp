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

float Rect::CenterX() const
{
	return (right + left) / 2;
}

float Rect::CenterY() const
{
	return (top + bottom) / 2;
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

Rect Rect::GetBoundingRect(const std::vector<Rect>& rects)
{
	ASSERT(rects.size() > 0);
	Rect rect = rects[0];

	for (int i = 1; i < rects.size(); i++)
	{
		if (rects[i].bottom > rect.bottom)
		{
			rect.bottom = rects[i].bottom;
		}

		if (rects[i].top < rect.top)
		{
			rect.top = rects[i].top;
		}

		if (rects[i].right > rect.right)
		{
			rect.right = rects[i].right;
		}

		if (rects[i].left > rect.left)
		{
			rect.left = rects[i].left;
		}
	}

	return rect;
}
