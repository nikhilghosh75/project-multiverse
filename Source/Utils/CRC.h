#pragma once
#include <stdint.h>

class CRC
{
public:
	static uint32_t Calculate(const void* data, size_t length);

private:
	static uint32_t CalculateRemainder(const void* data, size_t length);
};