#include "BinaryFileStream.h"

BinaryFileStream::BinaryFileStream()
{
	file = nullptr;
	hasBeenOpened = false;
	bigEndian = false;
	currentOffset = 0;
}

BinaryFileStream::BinaryFileStream(const char* filename)
{
	file = fopen(filename, "r");

	hasBeenOpened = file != nullptr;
	bigEndian = false;
	currentOffset = 0;
}

BinaryFileStream::BinaryFileStream(const char* filename, bool write)
{
	if (write)
		file = fopen(filename, "w");
	else
		file = fopen(filename, "r");

	hasBeenOpened = file != nullptr;
	bigEndian = false;
	currentOffset = 0;
}

BinaryFileStream::BinaryFileStream(const char* filename, bool write, FileFlags flags)
{
	if (write)
		file = fopen(filename, "w");
	else
		file = fopen(filename, "r");

	hasBeenOpened = file != nullptr;

	if ((flags & FileFlags::BigEndian) == FileFlags::BigEndian)
		bigEndian = true;
	else
		bigEndian = false;

	currentOffset = 0;
}

uint16_t BinaryFileStream::SwapEndian(uint16_t i)
{
	return (i << 8) | ((i >> 8));;
}

uint32_t BinaryFileStream::SwapEndian(uint32_t i)
{
	return ((i >> 24) & 0x000000FF) | ((i >> 8) & 0x0000FF00) | ((i << 8) & 0x00FF0000) | ((i << 24) & 0xFF000000);;
}

void BinaryFileStream::Read(void* bytes, uint32_t size)
{
	currentOffset += size;
	fread(bytes, size, 1, file);
}

void BinaryFileStream::Skip(uint32_t bytes)
{
	fseek(file, currentOffset + bytes, SEEK_SET);
	currentOffset += bytes;
}

void BinaryFileStream::ReverseSkip(uint32_t bytes)
{
	fseek(file, currentOffset - bytes, SEEK_SET);
	currentOffset -= bytes;
}

void BinaryFileStream::Sync()
{
	fseek(file, currentOffset, SEEK_SET);
}

BinaryFileStream& BinaryFileStream::operator>>(char& c)
{
	currentOffset += 1;
	c = (char)fgetc(file);
	return *this;
}

BinaryFileStream& BinaryFileStream::operator>>(unsigned char& c)
{
	currentOffset += 1;

	c = (unsigned char)fgetc(file);
	return *this;
}

BinaryFileStream& BinaryFileStream::operator>>(int16_t& i)
{
	currentOffset += 2;

	i = 0;
	char i1, i2;
	i1 = (char)fgetc(file);
	i2 = (char)fgetc(file);

	i = ((int16_t)i1 << 8) + (int16_t)i2;

	if (bigEndian)
	{
		i = (i << 8) | ((i >> 8) & 0x00FF);
	}
	return *this;
}

BinaryFileStream& BinaryFileStream::operator>>(uint16_t& i)
{
	currentOffset += 2;

	i = 0;
	unsigned char i1, i2;
	i1 = (unsigned char)fgetc(file);
	i2 = (unsigned char)fgetc(file);

	i = ((uint16_t)i1 << 8) + (uint16_t)i2;

	if (bigEndian)
	{
		i = (i << 8) | ((i >> 8));
	}
	return *this;
}

BinaryFileStream& BinaryFileStream::operator>>(int32_t& i)
{
	currentOffset += 4;

	i = 0;
	char i1, i2, i3, i4;
	i1 = (char)fgetc(file);
	i2 = (char)fgetc(file);
	i3 = (char)fgetc(file);
	i4 = (char)fgetc(file);

	i = ((int32_t)i1 << 24) + ((int32_t)i2 << 16) + ((int32_t)i3 << 8) + (int32_t)i4;

	if (bigEndian)
	{
		i = ((i >> 24) & 0x000000FF) | ((i >> 8) & 0x0000FF00) | ((i << 8) & 0x00FF0000) | ((i << 24) & 0xFF000000);
	}
	return *this;
}

BinaryFileStream& BinaryFileStream::operator>>(uint32_t& i)
{
	currentOffset += 4;

	i = 0;
	unsigned char i1, i2, i3, i4;
	i1 = (unsigned char)fgetc(file);
	i2 = (unsigned char)fgetc(file);
	i3 = (unsigned char)fgetc(file);
	i4 = (unsigned char)fgetc(file);

	i = ((uint32_t)i1 << 24) + ((uint32_t)i2 << 16) + ((uint32_t)i3 << 8) + (uint32_t)i4;

	if (bigEndian)
	{
		i = ((i >> 24) & 0x000000FF) | ((i >> 8) & 0x0000FF00) | ((i << 8) & 0x00FF0000) | ((i << 24) & 0xFF000000);
	}
	return *this;
}

BinaryFileStream& BinaryFileStream::operator>>(int64_t& i)
{
	currentOffset += 8;

	fread(&i, 8, 1, file);

	if (bigEndian)
	{
		i = ((i >> 56) & 0x00000000000000FF) | ((i >> 40) & 0x000000000000FF00) |
			((i >> 24) & 0x0000000000FF0000) | ((i >> 8) & 0x00000000FF000000) |
			((i << 8) & 0x000000FF00000000) | ((i << 24) & 0x0000FF0000000000) |
			((i << 40) & 0x00FF000000000000) | ((i << 56) & 0xFF00000000000000);
	}

	return *this;
}

BinaryFileStream& BinaryFileStream::operator>>(uint64_t& i)
{
	currentOffset += 8;

	fread(&i, 8, 1, file);

	if (!bigEndian)
	{
		i = ((i >> 56) & 0x00000000000000FF) | ((i >> 40) & 0x000000000000FF00) |
			((i >> 24) & 0x0000000000FF0000) | ((i >> 8) & 0x00000000FF000000) |
			((i << 8) & 0x000000FF00000000) | ((i << 24) & 0x0000FF0000000000) |
			((i << 40) & 0x00FF000000000000) | ((i << 56) & 0xFF00000000000000);
	}

	return *this;
}

BinaryFileStream& BinaryFileStream::operator>>(float& f)
{
	currentOffset += 4;

	fread(&f, 8, 1, file);

	return *this;
}

BinaryFileStream& BinaryFileStream::operator>>(double& d)
{
	currentOffset += 8;

	fread(&d, 8, 1, file);

	return *this;
}
