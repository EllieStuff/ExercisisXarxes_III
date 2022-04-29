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

	client->SetAddress("127.0.0.1");
	client->BindPort(Client_Initial_Port);

	std::cout << "Write your username" << std::endl;
	std::string clientName;
	std::cin >> clientName;
	client->SetName(clientName);
	Status status;

	while (!connect)
	{
		std::cout << " Connecting to the server" << std::endl;

		OutputMemoryStream* out = new OutputMemoryStream();
		out->WriteString("Hello_"+client->GetName());
		
		client->GetSocket()->Send(out, status, Server_Ip, Server_Port);

		if (status == Status::DONE)
			connect = true;
		else
			std::this_thread::sleep_for(std::chrono::seconds(2));
	}

	std::pair<IpAddress, unsigned short> _server;
	InputMemoryStream* inp = client->GetSocket()->Receive(status, _server);

	

	client->SetServerData(_server);
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