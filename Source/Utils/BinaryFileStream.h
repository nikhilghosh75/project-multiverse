#pragma once
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/*
A filestream class designed specifically for binary files.
*/

enum FileFlags
{
	LittleEndian = 1,
	BigEndian = 2,
};

inline FileFlags operator|(FileFlags a, FileFlags b)
{
	return static_cast<FileFlags>(static_cast<int>(a) | static_cast<int>(b));
}

inline FileFlags operator&(FileFlags a, FileFlags b)
{
	return static_cast<FileFlags>(static_cast<int>(a) & static_cast<int>(b));
}

class BinaryFileStream
{
public:
	BinaryFileStream();
	BinaryFileStream(const char* filename);
	BinaryFileStream(const char* filename, bool write);
	BinaryFileStream(const char* filename, bool write, FileFlags flags);

	uint32_t GetCurrentOffset() const { return currentOffset; }

	static uint16_t SwapEndian(uint16_t i);
	static uint32_t SwapEndian(uint32_t i);

	void Read(void* bytes, uint32_t size);

	void Skip(uint32_t bytes);
	void ReverseSkip(uint32_t bytes);

	void Sync();

	BinaryFileStream& operator>>(char& c);
	BinaryFileStream& operator>>(unsigned char& c);
	BinaryFileStream& operator>>(int16_t& i);
	BinaryFileStream& operator>>(uint16_t& i);
	BinaryFileStream& operator>>(int32_t& i);
	BinaryFileStream& operator>>(uint32_t& i);
	BinaryFileStream& operator>>(int64_t& i);
	BinaryFileStream& operator>>(uint64_t& i);
	BinaryFileStream& operator>>(float& f);
	BinaryFileStream& operator>>(double& d);

private:
	bool hasBeenOpened = false;
	bool bigEndian = false;

	uint32_t currentOffset;

	FILE* file;
};

