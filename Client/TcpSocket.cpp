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
	InputMemoryStream* input;
	sf::Packet packet;
	std::string charArray;
	sf::Socket::Status status = socket.receive(packet);

	_status = (Status)(int)status;

	if(_status != Status::DONE)
		return nullptr;

	packet >> charArray;
	input->Read(charArray.c_str());

	return input;
}

void TcpSocket::Send(OutputMemoryStream* _info, Status& _status)
{
	sf::Socket::Status status = socket.send(_info->GetBufferPtr(), _info->GetLength());
	_status = (Status)(int)status;
}
