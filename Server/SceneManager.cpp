#include "SceneManager.h"
#include "../res/IpAddress.h"
#include "../res/Utils.h"
#include <iostream>
#include <thread>

void SceneManager::EnterGame()
{

}

void SceneManager::UpdateGame()
{

}

SceneManager::SceneManager()
{
	gameState = State::INIT;
	game = new GameManager();
	Status status;
	game->sock.Bind(5000, status);

	if(status != Status::DONE) 
	{
		std::cout << "ERROR!!" << std::endl;
		gameState = State::END;
	}

	std::thread clientReceive(&SceneManager::ReceiveMessages, this);
	clientReceive.detach();
}

void UpdateInit() 
{

}

void SceneManager::ReceiveMessages()
{
	while (gameState != State::END)
	{
		Status status;

		std::pair<IpAddress, unsigned short> _client;
			
		InputMemoryStream* message = game->sock.Receive(status, _client);

		if(status == Status::DONE) 
		{
			std::string msg = message->ReadString();


			if(msg.find("hello")) 
			{
				bool clientExists = false;
				for (size_t i = 0; i < game->clients->size(); i++)
				{
					if (game->clients->at(i) == _client)
					{
						clientExists = true;
						break;
					}
				}
				if (!clientExists)
					game->clients->push_back(_client);
			}

			OutputMemoryStream* out = new OutputMemoryStream();
			out->WriteString("Welcome! " + game->GetClientId(_client));

			game->sock.Send(out, status, *_client.first.GetAddress(), _client.second);
		}
	}
}

SceneManager::~SceneManager()
{

}

void SceneManager::Update()
{
	while (gameState != State::END)
	{
	}
}