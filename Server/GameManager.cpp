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

int GameManager::GetClientId(std::pair<IpAddress, unsigned short> _client)
{
	for (size_t i = 0; i < clients->size(); i++)
	{
		if (clients->at(i) == _client)
			return i;
	}
	return -1;
}
