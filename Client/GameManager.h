#pragma once
#include "../res/UdpSocket.h"

class GameManager
{
	unsigned short port;
	IpAddress address;
	std::string userName; 
	UdpSocket sock;

	std::pair<IpAddress, unsigned short> serverData;
public:
	GameManager();
	GameManager(std::string _address, unsigned short _port);
	~GameManager();

	void Update();

	void SafeSend(OutputMemoryStream* out);

	void SetServerData(std::pair<IpAddress, unsigned short>);
	void SetServerData(IpAddress, unsigned short);

	std::pair<IpAddress, unsigned short> GetServerData() { return serverData; }

	UdpSocket* GetSocket() { return &sock; }

	void SetAddress(std::string);
	IpAddress GetAddress() { return address; }

	void BindPort(unsigned short _port);
	unsigned short GetPort() { return port; }

	void SetName(std::string _name) { userName = _name; }
	std::string GetName() { return userName; }
};