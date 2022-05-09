#include "SceneManager.h"
#include <iostream>

void SceneManager::EnterGame()
{

}

void SceneManager::UpdateGame()
{

}

SceneManager::SceneManager()
{
	gameState = State::INIT;
	client = new GameManager();
}

void SceneManager::UpdateInit()
{
	std::cout << "Write your username" << std::endl;
	std::string clientName;
	std::cin >> clientName;

	client->InitClient(clientName, "127.0.0.1");

	Status status;

	std::thread tReceive(&SceneManager::ReceiveMessages, this);
	tReceive.detach();

	while (!connected)
	{
		std::cout << " Connecting to the server" << std::endl;

		OutputMemoryStream* out = new OutputMemoryStream();
		out->Write((int)Commands::HELLO);
		out->WriteString(client->GetName());
		out->Write(client->GetSalt());
		
		client->GetSocket()->Send(out, status, Server_Ip, Server_Port);

		if (status != Status::DONE)
			continue;
	}
}

void SceneManager::ReceiveMessages()
{
	Status status;
	unsigned short _port;

	while(true) 
	{
		InputMemoryStream* in = client->GetSocket()->Receive(status, Server_Ip, _port);

		int command;
		in->Read(&command);

		Commands _com = (Commands)command;

		switch (_com)
		{
		case Commands::WELCOME:
			int id;
			in->Read(&id);
			client->SetClientID(id);

			int salt;
			in->Read(&salt);
			client->SetServerSalt(salt);

			*connected = true;
			break;
		case Commands::HELLO:
			break;
		case Commands::PLAYER_ID:
			break;
		case Commands::SALT:
			break;
		case Commands::CHALLENGE:
			break;
		}

		delete in;
	}
}

SceneManager::~SceneManager()
{

}

void SceneManager::Update()
{
	while (gameState != State::END)
	{
		UpdateInit();
	}
}