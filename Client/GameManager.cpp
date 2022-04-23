#include "GameManager.h"
#include "../res/Utils.h"

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
