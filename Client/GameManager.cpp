#include "GameManager.h"

GameManager::GameManager()
{

}

GameManager::GameManager(std::string _address, unsigned short _port)
{
	address = IpAddress(_address);
}

GameManager::~GameManager()
{

}

void GameManager::Update()
{

}
void GameManager::SetAddress(std::string _address)
{
	address.SetAddress(_address);
}

void GameManager::SafeSend(OutputMemoryStream* out)
{
	Status status;
	sock.Send(out, status, Server_Ip, Server_Port);
}

void GameManager::BindPort(unsigned short _port)
{
	Status status;
	
	sock.Bind(_port, status);
	while (status != Status::DONE)
	{
		_port++;
		sock.Bind(_port, status);
	}

	port = _port;
}

void GameManager::SetServerData(std::pair<IpAddress, unsigned short> _server)
{
	serverData = _server;
}

void GameManager::SetServerData(IpAddress _address, unsigned short _port)
{
	serverData.first = _address;
	serverData.second = _port;
}
