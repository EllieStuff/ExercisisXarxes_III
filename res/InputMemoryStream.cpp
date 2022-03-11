#include "InputMemoryStream.h"
#include <iostream>

InputMemoryStream::InputMemoryStream(char* _inBuffer, uint32_t _inByteCount) : mCapacity(_inByteCount), mHead(0)
{
	mBuffer = _inBuffer;
}

InputMemoryStream::~InputMemoryStream()
{
	
}

void InputMemoryStream::Read(void* _outData, uint32_t _inByteCount)
{
	//Despu�s de haber le�do, la posici�n del siguiente dato a leer quedar� en resultHead
	uint32_t resultHead = mHead + _inByteCount;
	//Si resultHead supera la capacidad del mBuffer, estamos leyendo m�s de lo que escribimos
	if (resultHead > mCapacity)
	{
		throw std::exception("InputMemoryStream::No data to read");
	}
	//Copio en _outData el tama�o de _outData contado desde mHead
	std::memcpy(_outData, mBuffer + mHead, _inByteCount);

	//Avanzo mHead para el siguiente Read.
	mHead = resultHead;
}

std::string InputMemoryStream::ReadString()
{
	int length;
	Read(&length);
	
	std::string str;
	str.resize(length);

	for (size_t i = 0; i < length; i++)
	{
		char a;
		Read(&a);
		str[i] = a;
	}

	return str;
}