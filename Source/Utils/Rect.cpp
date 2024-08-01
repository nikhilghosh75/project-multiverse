#include "Rect.h"

Rect::Rect()
	: top(0), bottom(0), left(0), right(0)
{
}

Rect::Rect(float _top, float _bottom, float _left, float _right)
	: top(_top), bottom(_bottom), left(_left), right(_right)
{
}

float Rect::Width() const
{
	return right - left;
}

float Rect::Height() const
{
	return top - bottom;
}
