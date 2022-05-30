#include "GameManager.h"

GameManager::GameManager()
{
	client = new ClientData();
}

GameManager::~GameManager()
{

}

void GameManager::Update()
{

}

void GameManager::SafeSend(OutputMemoryBitStream* out)
{
	Status status;
	sock.Send(out, status, Server_Ip, Server_Port);
}

void GameManager::BindPort(unsigned short &_OutPort)
{
	unsigned short _port = Client_Initial_Port;
	Status status;
	
	sock.Bind(_port, status);
	while (status != Status::DONE)
	{
		_port++;
		sock.Bind(_port, status);
	}

	_OutPort = _port;
}

void GameManager::InitClient(std::string _name, std::string _address)
{
	unsigned short port;

	BindPort(port);

	client->Start(port, _address, _name);
}
