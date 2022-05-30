#include "UdpSocket.h"

UdpSocket::UdpSocket()
{
}

UdpSocket::~UdpSocket()
{
}

void UdpSocket::Send(OutputMemoryBitStream* _out, Status& _status, IpAddress _address, unsigned short _port)
{
	_status = (Status)((int)sock.send(_out->GetBufferPtr(), _out->GetBitLength(), *_address.GetAddress(), _port));
}

InputMemoryBitStream* UdpSocket::Receive(Status& _status, std::pair<IpAddress, unsigned short> &_client)
{
	std::string* bufferChar = new std::string();
	size_t size = 2000;
	std::size_t received;
	InputMemoryBitStream* input = new InputMemoryBitStream((char*)bufferChar, size);

	_status = (Status)((int)sock.receive(bufferChar, size, received, *_client.first.GetAddress(), _client.second));

	return input;
}

InputMemoryBitStream* UdpSocket::Receive(Status& _status, IpAddress _address, unsigned short &_port)
{
	std::string* bufferChar = new std::string();
	size_t size = 2000;
	std::size_t received;
	InputMemoryBitStream* input = new InputMemoryBitStream((char*)bufferChar, size);

	_status = (Status)((int)sock.receive(bufferChar, size, received, *_address.GetAddress(), _port));

	return input;
}

void UdpSocket::Bind(unsigned int _port, Status& _status)
{
	_status = (Status)((int)sock.bind(_port));
}

void UdpSocket::Unbind()
{
	sock.unbind();
}
