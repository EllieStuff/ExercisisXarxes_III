#pragma once
#include <memory>
#include <vector>
#include <iostream>
#include <SFML/Network.hpp>
#include "Utils.h"
#include "InputMemoryStream.h"
#include "OutputMemoryStream.h"

class TcpSocket {
private:
	sf::TcpSocket socket;
	
public:
	TcpSocket();
	~TcpSocket();

	Status Connect(std::string _ip, unsigned short _port);
	void Disconnect() { socket.disconnect(); }

	InputMemoryStream* Receive(Status &_status);
	void Send(OutputMemoryStream* _info, Status& _status);
};