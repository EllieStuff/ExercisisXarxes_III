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

bool GameManager::ExistClient(int _id)
{
	return clients.find(_id) != clients.end();
}


Status GameManager::BindSocket()
{
	Status status;
	sock.Bind(Server_Port, status);
	return status;
}

Status GameManager::ReceiveMSG(InputMemoryStream* in, std::pair<IpAddress, unsigned short> client)
{
	Status status;

	in = sock.Receive(status, client.first, client.second);

	return status;
}

int GameManager::CreateClient(unsigned short _port ,IpAddress _address, std::string _name, int _salt)
{
	currentId++;
	clients[currentId] = new ClientData(_port, _address, _name, _salt);
	return currentId;
}

Status GameManager::SendClient(int _id, OutputMemoryStream* out)
{
	Status status;
	sock.Send(out, status, clients[_id]->GetAddress(), clients[_id]->GetPort());
	return status;
}


