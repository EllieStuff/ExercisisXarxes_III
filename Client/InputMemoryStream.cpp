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
	//Después de haber leído, la posición del siguiente dato a leer quedará en resultHead
	uint32_t resultHead = mHead + _inByteCount;
	//Si resultHead supera la capacidad del mBuffer, estamos leyendo más de lo que escribimos
	if (resultHead > mCapacity)
	{
		throw std::exception("InputMemoryStream::No data to read");
	}
	//Copio en _outData el tamaño de _outData contado desde mHead
	std::memcpy(_outData, mBuffer + mHead, _inByteCount);

	//Avanzo mHead para el siguiente Read.
	mHead = resultHead;
}

template<typename T>
inline void InputMemoryStream::Read(T* _outData)
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
inline void InputMemoryStream::Read(std::vector<T>* _outVector)
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

std::string InputMemoryStream::ReadString()
{
	//Recuperamos la longitud del string
	int length;
	Read(&length);
	//Reservamos memoria para un char* y lo recuperamos con la función genérica.
	char* buffer = new char[length + 1];
	Read(buffer, length);
	buffer[length] = '\0';

	//Construimos el string a partir del buffer
	std::string str = std::string(buffer);
	//Liberamos memoria
	delete[] buffer;
	return str;
}

//bool InputMemoryStream::StartListener(int _localPort)
//{
//	listener.listen(_localPort);
//	return true;
//}
//
//bool InputMemoryStream::Close()
//{
//	listener.close();
//	return true;
//}
//
//bool InputMemoryStream::ReceiveConnection(std::vector<sf::TcpSocket*>* _socks)
//{
//	sf::TcpSocket* sock = new sf::TcpSocket();
//	sf::Socket::Status status = listener.accept(*sock);
//
//	if (status == sf::Socket::Status::Done)
//	{
//		_socks->push_back(sock);
//		std::cout << "Connected with ip: " << sock->getRemoteAddress() << " and port: " << sock->getLocalPort() << std::endl;
//		return true;
//	}
//
//	return false;
//}
//
//bool InputMemoryStream::DisconnectClient(std::vector<sf::TcpSocket*>* _socks, sf::TcpSocket* _sock)
//{
//	for (auto it = _socks->begin(); it != _socks->end(); it++)
//	{
//		if (*it == _sock)
//		{
//			_socks->erase(it);
//			delete _sock;
//
//			return true;
//		}
//	}
//	return false;
//}
//
//std::string InputMemoryStream::ReceiveInfo(sf::TcpSocket* _sock)
//{
//	sf::Packet pack;
//	sf::Socket::Status status = _sock->receive(pack);
//	std::string msg;
//
//	if (status != sf::Socket::Done)
//	{
//		msg = "e";
//	}
//	else
//	{
//		pack >> msg;
//	}
//
//	return msg;
//}

