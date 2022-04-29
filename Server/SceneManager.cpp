#include "SceneManager.h"
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
	game->sock.Bind(Server_Port, status);
	game->clients = new std::vector<std::pair<IpAddress, unsigned short>>();

	if(status != Status::DONE) 
	{
		std::cout << "ERROR!!" << std::endl;
		gameState = State::END;
	}

	std::thread clientReceive(&SceneManager::ReceiveMessages, this);
	clientReceive.detach();
}

void SceneManager::UpdateInit() 
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

			if(msg.find("Hello_") != std::string::npos) 
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
			out->WriteString("Welcome_" + game->GetClientId(_client));

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