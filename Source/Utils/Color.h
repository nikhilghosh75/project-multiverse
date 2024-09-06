#pragma once
#include "glm/glm.hpp"
#include <stdint.h>

class Color
{
public:
	Color();
	Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
	Color(float r, float g, float b, float a);

	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;

	operator glm::vec4() const;
};

