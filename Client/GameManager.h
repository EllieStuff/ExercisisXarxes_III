#pragma once
#include "../res/UdpSocket.h"

class GameManager
{
	UdpSocket sock;

	IpAddress address;
	unsigned short port;

public:
	GameManager();
	GameManager(std::string _address, unsigned short _port);
	~GameManager();

	void Update();
};