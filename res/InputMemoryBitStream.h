#pragma once
#include <memory>
#include <string>


class InputMemoryBitStream
{
private:
	char*		mBuffer;
	uint32_t	mBitHead;
	uint32_t	mBitCapacity;

public:

	InputMemoryBitStream(char* _inBuffer, uint32_t _inBitCount) :
		mBitCapacity(_inBitCount),
		mBitHead(0)
	{
		mBuffer = _inBuffer;
	}

	~InputMemoryBitStream()
	{
	}

	const char*	GetBufferPtr() const
	{
		return mBuffer;
	}

	uint32_t GetRemainingBitCount()	const
	{
		return mBitCapacity - mBitHead;
	}

	void ReadBits(uint8_t* _outData, uint32_t _inBitCount)
	{

		uint32_t byteOffset = mBitHead >> 3;
		uint32_t bitOffset = mBitHead & 0x7;

		*_outData = static_cast< uint8_t >(mBuffer[byteOffset]) >> bitOffset;

		uint32_t bitsFreeThisByte = 8 - bitOffset;
		if (bitsFreeThisByte < _inBitCount)
		{
			*_outData |= static_cast< uint8_t >(mBuffer[byteOffset + 1]) << bitsFreeThisByte;
		}

		*_outData &= (~(0x00ff << _inBitCount));

		mBitHead += _inBitCount;
	}

	void ReadBits(void* _outData, uint32_t _inBitCount)
	{

		uint8_t* destByte = reinterpret_cast< uint8_t* >(_outData);
		int numByte = 0;
		while (_inBitCount > 8)
		{
			ReadBits(destByte, 8);
			_inBitCount -= 8;
			++destByte;
			numByte++;
		}
		if (_inBitCount > 0)
		{
			ReadBits(destByte, _inBitCount);
		}
	}

	void ReadBytes(void* _outData, uint32_t _inByteCount)
	{
		ReadBits(_outData, _inByteCount << 3);
	}

	
	template< typename T >
	void Read(T* _inData, uint32_t _inBitCount = sizeof(T) * 8)
	{
		*_inData = 1;
		static_assert(std::is_arithmetic< T >::value ||
			std::is_enum< T >::value,
			"Este read sólo soporta tipos básicos");
		ReadBits(_inData, _inBitCount);
	}

	std::string ReadString(uint32_t _inBitCount = sizeof(char) * 8)
	{
		int size;
		Read(&size, 5);

		char* _string = new char[size + 1];

		for (size_t i = 0; i < size; i++)
		{
			Read(&_string[i], _inBitCount);
		}

		_string[size] = '\0';

		return _string;
	}
	
};