#include "OutputMemoryStream.h"

void OutputMemoryStream::ReallocBuffer(uint32_t _newLength)
{
	//Redimensionamos el buffer --> ya no nos cabe todo lo que queremos copiar
	mBuffer = static_cast<char*>(std::realloc(mBuffer, _newLength));
	//TODO: Controlar error en realloc
	mCapacity = _newLength;
}

OutputMemoryStream::OutputMemoryStream() :mBuffer(0), mHead(0), mCapacity(0)
{
	//Inicialmente usamos un buffer de 32
	ReallocBuffer(32);
}

OutputMemoryStream::~OutputMemoryStream()
{
	//Pedimos memoria con realloc --> Liberamos con free.
	std::free(mBuffer);
}
void OutputMemoryStream::Write(const void* _inData, size_t _inByteCount)
{
	//Nos aseguramos de que hay espacio suficiente en mBuffer para copiar estos datos
	uint32_t resultHead = mHead + static_cast<uint32_t>(_inByteCount);
	if (resultHead > mCapacity)
	{
		//Si no hay espacio suficiente, pedimos m�s memoria
		uint32_t size = resultHead;
		if (resultHead < mCapacity * 2)
			size = mCapacity * 2;
		ReallocBuffer(size);
	}
	//Copiar en el buffer a partir de mHead
	std::memcpy(mBuffer + mHead, _inData, _inByteCount);
	//Incrementamos mHead para que el siguiente Write escriba a continuaci�n
	mHead = resultHead;
}
void OutputMemoryStream::WriteString(std::string _inString)
{
	//Escribo el tama�o del string y su informaci�n
	Write(_inString.size());
	Write(_inString.c_str(), _inString.size());
};

template<typename T>
inline void OutputMemoryStream::Write(T _data)
{
	//Tal y c�mo est� hecho, este Write s�lo funciona para tipos b�sicos.
	//As� evitamos que se nos cuele algo que se serializar� mal
	static_assert(std::is_arithmetic<T>::value || std::is_enum<T>::value, "Este Write solo soporta tipos basicos.");
	Write(&_data, sizeof(_data));
}

template<typename T>
void OutputMemoryStream::Write(const std::vector<T>& _inVector)
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