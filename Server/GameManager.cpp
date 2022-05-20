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
	return waitingClients.find(_id) != waitingClients.end();
}

void GameManager::ClientConnected(int _id)
{
	auto _client = waitingClients.find(_id);
	if (_client != waitingClients.end())
	{
		connectedClients.insert(*_client);
		waitingClients.erase(_id);
	}
}


Status GameManager::BindSocket()
{
	Status status;
	sock.Bind(Server_Port, status);
	return status;
}

InputMemoryStream* GameManager::ReceiveMSG(std::pair<IpAddress, unsigned short>* client, Status& status)
{
	InputMemoryStream* in = sock.Receive(status, client->first, client->second);

	return in;
}

int GameManager::CreateClient(unsigned short _port ,IpAddress _address, std::string _name, int _salt)
{
	currentId++;
	waitingClients[currentId] = new ClientData(_port, _address, _name, _salt);
	return currentId;
}

void GameManager::DeleteClient(int _id)
{
}

Status GameManager::SendClient(int _id, OutputMemoryStream* out)
{
	Status status;
	sock.Send(out, status, waitingClients[_id]->GetAddress(), waitingClients[_id]->GetPort());
	return status;
}

ClientData* GameManager::GetConnectedClient(int _id)
{
	if (connectedClients.find(_id) != connectedClients.end())
		return connectedClients[_id];
	else
		return nullptr;
}

ClientData* GameManager::GetConnectingClient(int _id)
{
	if (waitingClients.find(_id) != waitingClients.end())
		return waitingClients[_id];
	else
		return nullptr;
}


