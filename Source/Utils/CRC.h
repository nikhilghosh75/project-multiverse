#pragma once
#include <stdint.h>

namespace CRC
{
	uint32_t Calculate(const void* data, size_t length);
};