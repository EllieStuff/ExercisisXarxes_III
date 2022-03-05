#pragma once
#include <memory>
#include <vector>
#include <iostream>
#include <SFML/Network.hpp>
#include "Utils.h"


class TcpSocket {
private:
	sf::TcpSocket socket;

public:
	TcpSocket();
	~TcpSocket();

	Status Connect(std::string _ip, unsigned short _port);
	void Disconnect() { socket.disconnect(); }
	Status Receive();
	template<typename T>
	Status Send(T _msg);
};