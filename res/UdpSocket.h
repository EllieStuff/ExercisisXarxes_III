#pragma once
#include <SFML/Network.hpp>
#include "InputMemoryStream.h"
#include "OutputMemoryStream.h"
#include "Utils.h"

class UdpSocket
{
	sf::UdpSocket sock;

public:
	UdpSocket();
	~UdpSocket();

	void Send(OutputMemoryStream* out, Status& status);
	InputMemoryStream* Receive(Status &status);

	void Bind();
};