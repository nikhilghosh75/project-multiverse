#include "Color.h"

Color::Color()
	: r(0), g(0), b(0), a(255)
{
}

Color::Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
	: r(r), g(g), b(b), a(a)
{
}

Color::Color(float _r, float _g, float _b, float _a)
{
	r = static_cast<uint8_t>(_r * 255);
	g = static_cast<uint8_t>(_g * 255);
	b = static_cast<uint8_t>(_b * 255);
	a = static_cast<uint8_t>(_a * 255);
}

Color::operator glm::vec4() const
{
	return glm::vec4((float)r / 255.f, (float)g / 255.f, (float)b / 255.f, (float)a / 255.f);
}