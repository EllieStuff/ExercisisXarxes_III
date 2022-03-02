#pragma once
#pragma once
#include <memory>
#include <vector>
#include <math.h>
#include <string>

class OutputMemoryStream
{
private:
	char* mBuffer;
	uint32_t mHead;
	uint32_t mCapacity;
	void ReallocBuffer(uint32_t _newLength);
public:
	OutputMemoryStream();

	~OutputMemoryStream();

	char* GetBufferPtr() const { return mBuffer; }

	uint32_t GetLength() const 
	{
		//mHead con tiene la siguiente posici�n donde hay que copiar informaci�n
		//as� que es la longitud hasta donde hay informaci�n que nos interesa enviar.
		return mHead;
	}

	void Write(const void* _inData, size_t _inByteCount);

	template<typename T> void Write(T _data);

	template<typename T> void Write(const std::vector<T>& _inVector);

	//Es necesario hacer esta funci�n espec�fica 
	//para strings para evitar que entre en el Write con template gen�rico
	void WriteString(std::string _inString);
};