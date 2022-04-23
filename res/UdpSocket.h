#pragma once
#include <SFML/Network.hpp>
#include "InputMemoryStream.h"
#include "OutputMemoryStream.h"
#include "IpAddress.h"
#include "Utils.h"

class UdpSocket
{
	sf::UdpSocket sock;

public:
	UdpSocket();
	~UdpSocket();

	void Send(OutputMemoryStream* _out, Status& _status, IpAddress _address, unsigned int _port);
	InputMemoryStream* Receive(Status &_status, IpAddress _address, unsigned int _port);

	void Bind(unsigned int _port, Status& _status);
	void Unbind();
};