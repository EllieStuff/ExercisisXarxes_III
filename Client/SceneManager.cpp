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
	client = new GameManager();
}

void SceneManager::UpdateInit()
{
	bool connect = false;

	std::cout << "Write your username" << std::endl;
	std::string clientName;
	std::cin >> clientName;

	client->InitClient(clientName, "127.0.0.1");

	Status status;
	InputMemoryStream* in;

	while (!connect)
	{
		std::cout << " Connecting to the server" << std::endl;

		OutputMemoryStream* out = new OutputMemoryStream();
		out->Write((int)Commands::HELLO);
		out->Write(client->GetName());
		out->Write(client->GetSalt());
		
		client->GetSocket()->Send(out, status, Server_Ip, Server_Port);

		if (status != Status::DONE)
			continue;

		unsigned short _port;	//trash variable

		in = client->GetSocket()->Receive(status, Server_Ip, _port);


		int command;
		in->Read(&command);

		int id;
		in->Read(&id);
		client->SetClientID(id);

		int salt;
		in->Read(&salt);
		client->SetServerSalt(salt);

		connect = true;
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