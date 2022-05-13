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
	packetId = 0;
}

void SceneManager::SavePacketToTable(OutputMemoryStream* out, std::time_t time)
{
	auto endTime = std::chrono::system_clock::now();
	CriticalMessages message = CriticalMessages();
	message._ip = client->GetAddress().GetLocalAddress();
	message.port = client->GetPort();
	message.timeout = time;
	message.secondTimeout = std::chrono::system_clock::to_time_t(endTime);
	message.message = out;

	criticalMessages[packetId] = message;
	packetId++;
}

void SceneManager::CheckMessageTimeout()
{
	while(true) 
	{
		if (criticalMessages.size() == 0) return;

		Status status;
		for (auto it = criticalMessages.begin(); it != criticalMessages.end(); it++)
		{
			float time = it->second.timeout - it->second.secondTimeout;
			if (time > 1)
			{
				client->GetSocket()->Send(it->second.message, status, Server_Ip, Server_Port);
				it->second.timeout = 0;
			}
		}
	}
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

	std::thread tCheck(&SceneManager::CheckMessageTimeout, this);
	tCheck.detach();

	while (!connected)
	{
		std::cout << " Connecting to the server" << std::endl;

		OutputMemoryStream* out = new OutputMemoryStream();
		out->Write((int)Commands::HELLO);
		out->WriteString(client->GetName());
		out->Write(client->GetSalt());
		
		auto startTime = std::chrono::system_clock::now();
		client->GetSocket()->Send(out, status, Server_Ip, Server_Port);
		SavePacketToTable(out, std::chrono::system_clock::to_time_t(startTime));

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

			//*connected = true;
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