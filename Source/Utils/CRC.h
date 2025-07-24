#pragma once
#include <stdint.h>

namespace CRC
{
	// Uses CRC-32
	uint32_t Calculate(const void* data, size_t length);
};