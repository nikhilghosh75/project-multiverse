#include "BinaryBufferStream.h"

BinaryBufferStream::BinaryBufferStream()
	: buffer(nullptr), size(0), bigEndian(false), currentOffset(0)
{
}

BinaryBufferStream::BinaryBufferStream(void* buffer, size_t size)
	: buffer(buffer), size(size), bigEndian(false), currentOffset(0)
{
}

BinaryBufferStream::BinaryBufferStream(void* buffer, size_t size, FileFlags flags)
	: buffer(buffer), size(size), bigEndian(false), currentOffset(0)
{
	if ((flags & FileFlags::BigEndian) == FileFlags::BigEndian)
		bigEndian = true;
	else
		bigEndian = false;
}

void BinaryBufferStream::Read(void* bytes, uint32_t size)
{
	void* source = (char*)buffer + currentOffset;
	memcpy(bytes, source, size);

	currentOffset += size;
}

void BinaryBufferStream::Skip(uint32_t bytes)
{
	currentOffset += bytes;
}

void BinaryBufferStream::ReverseSkip(uint32_t bytes)
{
	currentOffset -= bytes;
}

void BinaryBufferStream::GoTo(uint32_t offset)
{
	currentOffset = offset;
}

BinaryBufferStream& BinaryBufferStream::operator>>(char& c)
{
	char* ptr = (char*)buffer + currentOffset;
	c = *ptr;
	currentOffset += 1;
	return *this;
}

BinaryBufferStream& BinaryBufferStream::operator>>(unsigned char& c)
{
	unsigned char* ptr = (unsigned char*)buffer + currentOffset;
	c = *ptr;
	currentOffset += 1;
	return *this;
}

BinaryBufferStream& BinaryBufferStream::operator>>(int16_t& i)
{
	char* address = (char*)buffer + currentOffset;
	int16_t* ptr = (int16_t*)address;
	i = *ptr;
	currentOffset += 2;

	if (bigEndian)
	{
		i = SwapEndian(i);
	}

	return *this;
}

BinaryBufferStream& BinaryBufferStream::operator>>(uint16_t& i)
{
	char* address = (char*)buffer + currentOffset;
	uint16_t* ptr = (uint16_t*)address;
	i = *ptr;
	currentOffset += 2;

	if (bigEndian)
	{
		i = SwapEndian(i);
	}

	return *this;
}

BinaryBufferStream& BinaryBufferStream::operator>>(int32_t& i)
{
	char* address = (char*)buffer + currentOffset;
	i = *((int32_t*)address);
	currentOffset += 4;

	if (bigEndian)
	{
		i = SwapEndian(i);
	}

	return *this;
}

BinaryBufferStream& BinaryBufferStream::operator>>(uint32_t& i)
{
	char* address = (char*)buffer + currentOffset;
	i = *((uint32_t*)address);
	currentOffset += 4;

	if (bigEndian)
	{
		i = SwapEndian(i);
	}

	return *this;
}

BinaryBufferStream& BinaryBufferStream::operator>>(int64_t& i)
{
	char* address = (char*)buffer + currentOffset;
	i = *((int64_t*)address);
	currentOffset += 8;
	return *this;
}

BinaryBufferStream& BinaryBufferStream::operator>>(uint64_t& i)
{
	char* address = (char*)buffer + currentOffset;
	i = *((int64_t*)address);
	currentOffset += 8;
	return *this;
}

BinaryBufferStream& BinaryBufferStream::operator>>(float& f)
{
	char* address = (char*)buffer + currentOffset;
	f = *((float*)address);
	currentOffset += 4;
	return *this;
}

BinaryBufferStream& BinaryBufferStream::operator>>(double& d)
{
	char* address = (char*)buffer + currentOffset;
	d = *((double*)address);
	currentOffset += 8;
	return *this;
}

int16_t BinaryBufferStream::SwapEndian(int16_t i)
{
	return (i << 8) | ((i >> 8) & 0xFF);
}

uint16_t BinaryBufferStream::SwapEndian(uint16_t i)
{
	return (i << 8) | ((i >> 8));
}

int32_t BinaryBufferStream::SwapEndian(int32_t i)
{
	return ((i >> 24) & 0x000000FF) | ((i >> 8) & 0x0000FF00) | ((i << 8) & 0x00FF0000) | ((i << 24) & 0xFF000000);

}

uint32_t BinaryBufferStream::SwapEndian(uint32_t i)
{
	return ((i >> 24) & 0x000000FF) | ((i >> 8) & 0x0000FF00) | ((i << 8) & 0x00FF0000) | ((i << 24) & 0xFF000000);
}
