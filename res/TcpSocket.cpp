#include "TcpSocket.h"

TcpSocket::TcpSocket()
{
}

TcpSocket::~TcpSocket()
{
}

Status TcpSocket::Connect(std::string _ip, unsigned short _port)
{
	sf::Socket::Status status = socket.connect(_ip, _port);

	return (Status)(int)(status);
}

InputMemoryStream* TcpSocket::Receive(Status& _status)
{
	std::string* bufferChar = new std::string();
	size_t size = 1000;
	std::size_t received;
	InputMemoryStream* input = new InputMemoryStream((char*)bufferChar, size);
	sf::Socket::Status status = socket.receive(bufferChar, size, received);

	_status = (Status)(int)status;
	std::cout << std::endl;

	if (_status == Status::DONE)
		return input;
	else
		return nullptr;
}

void TcpSocket::Send(OutputMemoryStream* _info, Status& _status)
{
	sf::Socket::Status status = socket.send(_info->GetBufferPtr(), _info->GetLength());
	_status = (Status)(int)status;
	std::cout << std::endl;
}

unsigned short TcpSocket::GetLocalPort()
{
	return socket.getLocalPort();
}

std::string TcpSocket::GetRemoteAddress()
{
	return socket.getRemoteAddress().toString();
}

unsigned short TcpSocket::GetRemotePort()
{
	return socket.getRemotePort();
}

bool TcpSocket::IsBlocking()
{
	return socket.isBlocking();
}

void TcpSocket::SetBlocking(bool _blocking)
{
	socket.setBlocking(_blocking);
}
