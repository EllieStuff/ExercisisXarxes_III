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
	int localPort;

	void ReallocBuffer(uint32_t _newLength);

public:
	OutputMemoryStream();

	~OutputMemoryStream();

	char* GetBufferPtr() const { return mBuffer; }

	uint32_t GetLength() const 
	{
		//mHead contiene la siguiente posici�n donde hay que copiar informaci�n
		//as� que es la longitud hasta donde hay informaci�n que nos interesa enviar.
		return mHead;
	}

	void Write(const void* _inData, size_t _inByteCount);

	template<typename T>
	inline void Write(T _data)
	{
		//Tal y c�mo est� hecho, este Write s�lo funciona para tipos b�sicos.
		//As� evitamos que se nos cuele algo que se serializar� mal
		static_assert(std::is_arithmetic<T>::value || std::is_enum<T>::value, "Este Write solo soporta tipos basicos.");
		Write(&_data, sizeof(_data));
	}

	template<typename T>
	inline void Write(const std::vector<T>& _inVector)
	{
		//Para serializar vectores, pasamos el tama�o del vector
		//y a continuaci�n todos sus elementos.
		size_t elementCount = _inVector.size();
		Write(elementCount);
		for (const T& element : _inVector)
		{
			Write(element);
		}
	}

	//Es necesario hacer esta funci�n espec�fica 
	//para strings para evitar que entre en el Write con template gen�rico
	void WriteString(std::string _inString);

};