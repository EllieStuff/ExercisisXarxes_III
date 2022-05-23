#include "SceneManager.h"
#include <iostream>
#include <SDL2/include/SDL.h>

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

	GamePlayerInfo game = GamePlayerInfo(0, 0);
	players->insert(std::pair<int, GamePlayerInfo>(client->GetClientID(), game));

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

		SDL_Init(SDL_INIT_EVERYTHING);
		SDL_Window* window = NULL;
		window = SDL_CreateWindow
		(
			"Game Test Networking", SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			640,
			480,
			SDL_WINDOW_SHOWN
		);

		SDL_Renderer* renderer = NULL;
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);


		while (true) 
		{
			SDL_Event _event;
			auto _player = players->find(client->GetClientID());
			int posX = _player->second.pos.x;
			int posY = _player->second.pos.y;

			if(SDL_PollEvent(&_event) != 0) 
			{
				switch (_event.key.keysym.sym) {
				case SDLK_LEFT:
					posX -= 1;
					break;
				case SDLK_RIGHT:
					posX += 1;
					break;
				case SDLK_UP:
					posY -= 1;
					break;
				case SDLK_DOWN:
					posY += 1;
					break;
				default:
					break;
				}
			}

			SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

			SDL_RenderClear(renderer);


			SDL_Rect playerLocal;
			SDL_Rect r;
			r.x = posX;
			r.y = posY;
			r.w = 50;
			r.h = 50;

			SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);

			SDL_RenderFillRect(renderer, &r);

			_player->second.SetPlayerPos(posX, posY);

			for (auto it = players->begin(); it != players->end(); it++)
			{
				if (it->first == client->GetClientID()) continue;

				SDL_Rect playerLocal;
				SDL_Rect r2;
				r2.x = it->second.pos.x;
				r2.y = it->second.pos.y;
				r2.w = 50;
				r2.h = 50;

				SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);

				SDL_RenderFillRect(renderer, &r2);

			}

			SDL_RenderPresent(renderer);

			OutputMemoryStream* out = new OutputMemoryStream();

			out->Write((int)Commands::UPDATE_GAME);
			out->Write(client->GetClientID());

			out->Write(posX);
			out->Write(posY);

			Status status;

			client->GetSocket()->Send(out, status, Server_Ip, Server_Port);

			delete out;
		}

		break;
	}

	case 2: 
	{
		OutputMemoryStream* out = new OutputMemoryStream();
		out->Write((int)Commands::EXIT);
		out->Write(client->GetClientID());

		Status status;
		client->GetSocket()->Send(out, status, Server_Ip, Server_Port);
		delete out;

		exit(0);
		break;
	}
	}
}

SceneManager::SceneManager()
{
	criticalMessages = new std::map<Commands, CriticalMessages>();
	players = new std::map<int, GamePlayerInfo>();

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

		while(*pong != true) 
		{
			auto endTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
			int time = endTime - startTime;

			if (*match) break;

			if(time > 5)
			{
				std::cout << "Disconnected!!!!!" << std::endl;
				exit(0);
			}
		}

		delete out;

		if (*match) break;

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
			
		//--------------- Disconnection ---------------
		case Commands::PING_PONG:
			{
				pong = new bool(true);
			}
			break;
		case Commands::EXIT:
			{
				exit(0);
			}
			break;
		//--------------- Disconnection ---------------
			
		//--------------- Ingame Receives -----------
		case Commands::MATCH_FOUND:
			{
				std::cout << "New Player Joined!" << std::endl;
				in->Read(&matchID);
				int playerID;
				in->Read(&playerID);
				match = new bool(true);
			}
			break;
		//--------------- Ingame Receives -----------

		case Commands::UPDATE_GAME:
			{
				int playerID;
				in->Read(&playerID);
				float posX;
				float posY;
				in->Read(&posX);
				in->Read(&posY);

				auto _player = players->find(playerID);
				if(_player == players->end())
				{
					mtx.lock();
					GamePlayerInfo game = GamePlayerInfo(0, 0);
					players->insert(std::pair<int, GamePlayerInfo>(playerID, game));
					mtx.unlock();
				}
				
				_player->second.SetPlayerPos(posX, posY);

				std::cout << "PLAYER: " << std::to_string(playerID) << std::endl;
				std::cout << _player->second.pos.x << std::endl;
				std::cout << _player->second.pos.y << std::endl;
				std::cout << "_______________________________________" << std::endl;

			}
			break;
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