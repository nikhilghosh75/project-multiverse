#pragma once

#include "FixedPoint.h"
#include "FileUtils.h"

/*
* A class that exists for when a buffer is already loaded but we want to go through it like a filestream
*/
class BinaryBufferStream
{
public:
	BinaryBufferStream();
	BinaryBufferStream(void* buffer, size_t size);
	BinaryBufferStream(void* buffer, size_t size, FileFlags flags);

	uint32_t GetCurrentOffset() const { return currentOffset; }

	void Read(void* bytes, uint32_t size);

	void Skip(uint32_t bytes);
	void ReverseSkip(uint32_t bytes);

	void GoTo(uint32_t offset);

	BinaryBufferStream& operator>>(char& c);
	BinaryBufferStream& operator>>(unsigned char& c);
	BinaryBufferStream& operator>>(int16_t& i);
	BinaryBufferStream& operator>>(uint16_t& i);
	BinaryBufferStream& operator>>(int32_t& i);
	BinaryBufferStream& operator>>(uint32_t& i);
	BinaryBufferStream& operator>>(int64_t& i);
	BinaryBufferStream& operator>>(uint64_t& i);
	BinaryBufferStream& operator>>(float& f);
	BinaryBufferStream& operator>>(double& d);

	template<typename T, int DecimalBytes>
	BinaryBufferStream& operator>>(FixedPoint<T, DecimalBytes>& f)
	{
		T value;
		*this >> value;
		
		f = FixedPoint<T, DecimalBytes>(value);

		return *this;
	}

private:
	static int16_t SwapEndian(int16_t i);
	static uint16_t SwapEndian(uint16_t i);
	static int32_t SwapEndian(int32_t i);
	static uint32_t SwapEndian(uint32_t i);

	uint32_t currentOffset;
	bool bigEndian = false;

	void* buffer;
	size_t size;
};