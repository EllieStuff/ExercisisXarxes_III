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
	
public:
	sf::TcpSocket socket;
	
	TcpSocket();
	~TcpSocket();

	Status Connect(std::string _ip, unsigned short _port);
	void Disconnect() { socket.disconnect(); }

	InputMemoryStream* Receive(Status &_status);
	void Send(OutputMemoryStream* _info, Status& _status);

	unsigned short GetLocalPort();
	std::string GetRemoteAddress();
	unsigned short GetRemotePort();

	bool IsBlocking();
	void SetBlocking(bool _blocking);


};