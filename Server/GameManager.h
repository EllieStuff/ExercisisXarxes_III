#pragma once
#include "../res/UdpSocket.h"

class UdpSocket;
enum class Status;
class IpAdress;

class GameManager
{

public:
	unsigned short port;
	IpAddress address;
	std::string userName;
	std::vector<std::pair<IpAddress, unsigned short>>* clients;
	UdpSocket sock;
	GameManager();
	GameManager(std::string _address, unsigned short _port);
	~GameManager();

	void Update();
	int GetClientId(std::pair<IpAddress, unsigned short >);
};