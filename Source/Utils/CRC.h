#pragma once
#include <stdint.h>

namespace CRC
{
	static uint32_t Calculate(const void* data, size_t length);
};