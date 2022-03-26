#pragma once
#include <SFML/Network.hpp>
#include "TcpListener.h"
#include "TcpSocket.h"


class Selector
{
	sf::SocketSelector selector;

public:
	Selector();
	~Selector();

	void Add(TcpSocket* sock);
	void Add(TcpListener* listener);

	void Remove(TcpSocket* sock);
	void Remove(TcpListener* listener);

	void Clear();

	bool Wait();
	bool IsReady(TcpSocket* sock);
	bool IsReady(TcpListener* listener);
};