#pragma once
#include <SFML/Network.hpp>
#include "InputMemoryBitStream.h"
#include "OutputMemoryBitStream.h"
#include "IpAddress.h"
#include "Utils.h"

class UdpSocket
{
	sf::UdpSocket sock;

public:
	UdpSocket();
	~UdpSocket();

	void Send(OutputMemoryBitStream* _out, Status& _status, IpAddress _address, unsigned short _port);
	InputMemoryBitStream* Receive(Status& _status, std::pair<IpAddress, unsigned short> &_client);
	InputMemoryBitStream* Receive(Status& _status, IpAddress _address, unsigned short& _port);

	void Bind(unsigned int _port, Status& _status);
	void Unbind();
};