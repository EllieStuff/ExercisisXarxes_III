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

	struct PeerAddress {
		std::string ip;
		unsigned short port;
	};

public:

	InputMemoryStream(char* _inBuffer, uint32_t _inByteCount);

	~InputMemoryStream();

	void Read(void* _outData, uint32_t _inByteCount);

	template<typename T>
	inline void Read(T* _outData)
	{
		//Tal y cómo está hecho, este Read sólo funciona para tipos básicos.
		//Así evitamos deserializar algo que no se hará bien
		static_assert(std::is_arithmetic<T>::value || std::is_enum<T>::value, "Este Read solo soporta tipos basicos.");
		//En sizeOf se guarda el tamaño de este tipo en bytes
		//Si es un char: 1
		//Si es un int: 4
		int sizeOf = sizeof(*_outData);
		Read(_outData, sizeOf);
	}

	template<typename T>
	inline void Read(std::vector<T>* _outVector)
	{
		//Cuando recuperamos un vector, lo primero que nos llega es el número de posiciones.
		int elementCount;
		Read(&elementCount);
		//Recuperamos todas las posiciones y las copiamos en el vector.
		for (size_t i = 0; i < elementCount; i++)
		{
			T element;
			Read(&element);
			_outVector->push_back(element);
		}

	}

	//Es necesario hacer esta función específica para strings 
	//para evitar que entre en el Read con template genérico
	std::string ReadString();

};