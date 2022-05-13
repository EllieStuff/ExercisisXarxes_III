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
	status = game->BindSocket();

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
		Status status = Status::NOT_READY;

		std::pair<IpAddress*, unsigned short*>* _client = new std::pair<IpAddress*, unsigned short*>;

		std::string ip = "127.0.0.1";
		_client->first = new IpAddress(ip);
		_client->second = new unsigned short;

		InputMemoryStream* message = game->ReceiveMSG(_client, status);

		if(status == Status::DONE) 
		{
			Commands commandNum;
			int num;

			message->Read(&num);

			commandNum = (Commands) num;

			switch (commandNum)
			{
			case Commands::WELCOME:
				break;
			case Commands::HELLO: 
				{
					OutputMemoryStream* out = new OutputMemoryStream();
					std::string name = message->ReadString();
					int salt;
					message->Read(&salt);

					int id = game->CreateClient(*_client->second, *_client->first, name, salt);
					out->Write(Commands::CHALLENGE);
					out->Write(id);
					out->Write(game->GetSalt(id));

					game->SendClient(id, out);
				}
				break;
			case Commands::PLAYER_ID:
				break;
			case Commands::SALT:
				{
					int id;
					int salt;
					message->Read(&id);
					message->Read(&salt);

					OutputMemoryStream* out = new OutputMemoryStream();

					if(game->GetSalt(id) == salt) 
					{
						out->Write((int) Commands::WELCOME);
						out->WriteString("Welcome!");
					}
					else 
					{
						out->Write((int) Commands::SALT);
					}

					game->SendClient(id, out);
				}
				break;
			case Commands::CHALLENGE:
				break;
			}
		}
		else 
		{
			delete message;
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
		ReceiveMessages();
	}
}