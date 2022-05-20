#include "SceneManager.h"
#include <iostream>

void SceneManager::EnterGame()
{

}

void SceneManager::UpdateGame()
{
	std::cout << "1. Matchmaking" << std::endl;
	std::cout << "2. Exit" << std::endl;
	std::cout << "OPTION: ";
	int option;
	std::cin >> option;

	std::cout << "" << std::endl;

	switch (option)
	{
	case 1:
	{
		std::cout << "1. Create match" << std::endl;
		std::cout << "2. Join match" << std::endl;

		int option;

		std::cin >> option;

		OutputMemoryStream* out = new OutputMemoryStream();
		out->Write((int)Commands::SEARCH_MATCH);
		out->Write(client->GetClientID());
		
		if(option == 1)
			out->Write(false);
		if (option == 2)
			out->Write(true);

		Status status;

		client->GetSocket()->Send(out, status, Server_Ip, Server_Port);

		std::cout << "Waiting for match" << std::endl;

		while (!(*match)) { }

		std::cout << "Match Found!" << std::endl;

		while (true) {  }

		break;
	}

	case 2: 
	{
		exit(0);
		break;
	}
	}
}

SceneManager::SceneManager()
{
	criticalMessages = new std::map<Commands, CriticalMessages>();

	pong = new bool(false);

	gameState = State::INIT;
	client = new GameManager();
	connected = new bool(false);
	packetId = 0;
}

void SceneManager::SavePacketToTable(Commands _packetId, OutputMemoryStream* out, std::time_t time)
{
	CriticalMessages message = CriticalMessages(client->GetAddress().GetLocalAddress(), client->GetPort(), time, out);

	auto mapPosition = criticalMessages->find(_packetId);

	if (mapPosition != criticalMessages->end())
	{
		delete mapPosition->second.message;
		mapPosition->second = message;
	}
	else
	{
		criticalMessages->insert(std::pair<Commands, CriticalMessages>(_packetId, message));
	}
}

void SceneManager::Ping() 
{
	while(true) 
	{
		*pong = false;
		OutputMemoryStream* out = new OutputMemoryStream();
		out->Write((int)Commands::PING_PONG);
		out->Write(client->GetClientID());

		unsigned short port;

		Status status;

		auto startTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

		client->GetSocket()->Send(out, status, Server_Ip, Server_Port);
		delete out;

		while(*pong != true) 
		{
			auto endTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
			int time = endTime - startTime;
			if(time > 5)
			{
				std::cout << "Disconnected!!!!!" << std::endl;
				exit(0);
			}
		}

		std::this_thread::sleep_for(std::chrono::seconds(2));
	}
}

void SceneManager::CheckMessageTimeout()
{
	while(true) 
	{
		if (criticalMessages->size() == 0) continue;
		auto currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

		mtx.lock();

		Status status;
		for (auto it = criticalMessages->begin(); it != criticalMessages->end(); it++)
		{
			float time = currentTime - it->second.startTime;
			if (time > 1)
			{
				client->GetSocket()->Send(it->second.message, status, Server_Ip, Server_Port);
				it->second.startTime = currentTime;
			}
		}

		mtx.unlock();
	}
}

void SceneManager::MessageReceived(Commands _message)
{
	mtx.lock();
	auto mapPosition = criticalMessages->find(_message);

	if (mapPosition != criticalMessages->end())
	{
		delete mapPosition->second.message;
		criticalMessages->erase(mapPosition);
	}
	mtx.unlock();
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


	std::cout << " Connecting to the server" << std::endl;

	OutputMemoryStream* out = new OutputMemoryStream();
	out->Write((int)Commands::HELLO);
	out->WriteString(client->GetName());
	out->Write(client->GetClientSalt());
		
	auto startTime = std::chrono::system_clock::now();
	client->GetSocket()->Send(out, status, Server_Ip, Server_Port);
	SavePacketToTable(Commands::HELLO, out, std::chrono::system_clock::to_time_t(startTime));

	while (!(*connected)) { }

	std::thread tPing(&SceneManager::Ping, this);
	tPing.detach();

	gameState = State::GAME;
}

void SceneManager::ReceiveMessages()
{
	Status status;
	unsigned short _port;
	match = new bool(false);

	while(true) 
	{
		InputMemoryStream* in = client->GetSocket()->Receive(status, Server_Ip, _port);

		int command;
		in->Read(&command);

		Commands _com = (Commands)command;

		switch (_com)
		{
		//---------------Connection---------------
		case Commands::WELCOME:
			{
				std::cout << "Welcome! " << client->GetName() << std::endl;

				float rttKey;
				in->Read(&rttKey);

				OutputMemoryStream* out = new OutputMemoryStream();
				
				out->Write((int)Commands::ACK_WELCOME);
				out->Write(client->GetClientID());
				out->Write(rttKey);

				client->GetSocket()->Send(out, status, Server_Ip, Server_Port);
				MessageReceived(Commands::SALT);

				connected = new bool(true);
			}
			break;
		case Commands::SALT:
			{
				std::cout << "Salt error" << std::endl;

				float rttKey;
				in->Read(&rttKey);

				OutputMemoryStream* out = new OutputMemoryStream();

				out->Write((int) Commands::SALT);
				out->Write(client->GetClientID());
				out->Write(rttKey);

				int _result = client->GetServerSalt() & client->GetClientSalt();

				out->Write(_result);

				auto startTime = std::chrono::system_clock::now();
				client->GetSocket()->Send(out, status, Server_Ip, Server_Port);
				SavePacketToTable(Commands::SALT, out, std::chrono::system_clock::to_time_t(startTime));
			}
			break;
		case Commands::CHALLENGE:
			{
				std::cout << "Challenge operation" << std::endl;
				
				float rttKey;
				in->Read(&rttKey);

				int id;
				in->Read(&id);
				client->SetClientID(id);

				int salt;
				in->Read(&salt);
				client->SetServerSalt(salt);

				OutputMemoryStream* out = new OutputMemoryStream();

				int _result = salt & client->GetClientSalt();

				std::cout << "Server SALT: " << client->GetServerSalt() << ", Client SALT: " << client->GetClientSalt() << ", Result: " << _result << std::endl;
				
				out->Write((int) Commands::SALT);
				out->Write(id);
				out->Write(rttKey);
				out->Write(_result);

				auto startTime2 = std::chrono::system_clock::now();
				client->GetSocket()->Send(out, status, Server_Ip, Server_Port);
				SavePacketToTable(Commands::SALT, out, std::chrono::system_clock::to_time_t(startTime2));
				MessageReceived(Commands::HELLO);
			}
			break;
		//--------------- Connection ---------------
			
		//--------------- Ping-Pong ---------------
		case Commands::PING_PONG:
			{
				pong = new bool(true);
			}
			break;
		//--------------- Ping-Pong ---------------
			
		//--------------- Ingame Receives -----------
		case Commands::MATCH_FOUND:
			{
				std::cout << "New Player Joined!" << std::endl;
				int matchID;
				in->Read(&matchID);
				match = new bool(true);
			}
			break;
		//--------------- Ingame Receives -----------
		}

		delete in;
	}
}

SceneManager::~SceneManager()
{
	delete connected;
}

void SceneManager::Update()
{
	while (gameState != State::END)
	{
		switch (gameState)
		{
		case SceneManager::State::INIT:
			UpdateInit();
			break;
		case SceneManager::State::GAME:
			UpdateGame();
			break;
		}
	}
}