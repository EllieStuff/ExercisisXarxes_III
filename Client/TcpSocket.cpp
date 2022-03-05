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
	char* bufferChar = new char();
	size_t size = 1000;
	std::size_t received;
	InputMemoryStream* input = new InputMemoryStream(bufferChar, size);
	void* bufferVoid = bufferChar;
	sf::Socket::Status status = socket.receive(bufferVoid, size, received);

	_status = (Status)(int)status;

	if(_status != Status::DONE)
		return nullptr;

	input->Read(bufferVoid, received);

	return input;
}

void TcpSocket::Send(OutputMemoryStream* _info, Status& _status)
{
	sf::Socket::Status status = socket.send(_info->GetBufferPtr(), _info->GetLength());
	_status = (Status)(int)status;
}
