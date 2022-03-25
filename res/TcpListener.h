#pragma once
#include <memory>
#include <vector>
#include <iostream>
#include <SFML/Network.hpp>
#include "Utils.h"

class TcpSocket;

class TcpListener {
private:

public:
	sf::TcpListener listener;
	TcpListener();
	~TcpListener();

	sf::SocketSelector selector;

	Status Accept(TcpSocket &_socket);
	void Close();

	Status Listen(unsigned short _port);

	unsigned short GetLocalPort();

	bool IsBlocking();
	void SetBlocking(bool _blocking);


};