#include "CRC.h"

uint32_t CRC::Calculate(const void* data, size_t length)
{
	uint32_t crc = 0xFFFFFFFF;
	const unsigned char* bytes = (unsigned char*)data;

	for (size_t i = 0; i < length; i++)
	{
		crc ^= bytes[i];
		for (size_t j = 0; j < 8; j++)
		{
			if (crc & 1)
			{
				crc = (crc >> 1) ^ 0xEDB88320;
			}
			else
			{
				crc >>= 1;
			}
		}
	}

	return ~crc;
}
