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
	InputMemoryStream* packet;
	//sf::Socket::Status status = socket.receive();

	return nullptr;
}

void TcpSocket::Send(OutputMemoryStream* _info, Status& _status)
{
}
