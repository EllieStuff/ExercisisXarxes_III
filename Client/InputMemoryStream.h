#pragma once
#include <memory>
#include <vector>
#include <string>

class InputMemoryStream
{
private:
	char* mBuffer;
	uint32_t mHead;
	uint32_t mCapacity;
public:
	InputMemoryStream(char* _inBuffer, uint32_t _inByteCount);

	~InputMemoryStream();

	void Read(void* _outData, uint32_t _inByteCount);

	template<typename T> void Read(T* _outData);

	template<typename T> void Read(std::vector<T>* _outVector);

	//Es necesario hacer esta función específica para strings 
	//para evitar que entre en el Read con template genérico
	std::string ReadString();
};