#pragma once
#include "Rect.h"
#include <functional>

class Button
{
public:
	static void Add(Rect rect, std::function<void()> callback);
};