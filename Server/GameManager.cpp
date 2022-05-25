#include "GameManager.h"
#include <mutex>

GameManager::GameManager()
{

}

GameManager::GameManager(std::string _address, unsigned short _port)
{
	address = IpAddress(_address);
}

GameManager::~GameManager()
{
	for (auto _client = connectedClients.begin(); _client != connectedClients.end(); _client++)
	{
		delete _client->second;
	}

	for (auto _client = waitingClients.begin(); _client != waitingClients.end(); _client++)
	{
		delete _client->second;
	}
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


void GameManager::DisconnectClient(int _id)
{
	if(!waitingClients[_id]->disconnected) 
	{
		waitingClients[_id]->disconnected = true;
		return;
	}

	if (GetConnectingClient(_id) != nullptr)
	{
		delete waitingClients[_id];
		waitingClients.erase(_id);
	}
	else if (GetConnectedClient(_id) != nullptr)
	{
		delete connectedClients[_id];
		connectedClients.erase(_id);
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

void GameManager::SendAll(OutputMemoryStream* out)
{
	Status status;
	for (auto _client = connectedClients.begin(); _client != connectedClients.end(); _client++)
	{
		sock.Send(out, status, _client->second->GetAddress(), _client->second->GetPort());
	}

	for (auto _client = waitingClients.begin(); _client != waitingClients.end(); _client++)
	{
		sock.Send(out, status, _client->second->GetAddress(), _client->second->GetPort());
	}
}

ClientData* GameManager::GetClient(int _id)
{
	ClientData* _client = GetConnectedClient(_id);
	if (_client != nullptr) return _client;

	_client = GetConnectingClient(_id);
	if (_client != nullptr) return _client;

	return nullptr;
}

ClientData* GameManager::GetConnectedClient(int _id)
{
	if (waitingClients.find(_id) != waitingClients.end())
		return waitingClients[_id];
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


