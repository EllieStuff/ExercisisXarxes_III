#include "SceneManager.h"
#include <iostream>
#include <SDL2/include/SDL.h>

void SceneManager::EnterGame()
{

}

void SceneManager::UpdateGame()
{

	while(*gameState != State::EXIT) 
	{
		players->clear();
		*match = false;
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

			OutputMemoryStream* out = new OutputMemoryStream();
			out->Write((int)Commands::SEARCH_MATCH);
			out->Write(client->GetClientID());

			Status status;

			client->GetSocket()->Send(out, status, Server_Ip, Server_Port);

			std::cout << "Waiting for match" << std::endl;

			while (!(*match)) { std::this_thread::sleep_for(std::chrono::seconds(1)); }
			*gameState = State::GAME;

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

			float timeMargin = 0.01 * CLOCKS_PER_SEC;
			time_t startTime = std::clock();

			while (*gameState != State::END)
			{
				//startTime += 0.2f;
				time_t currTime = std::clock();
				if (currTime - startTime > timeMargin && accumulatedMessages.size() > 0)
				{
					startTime = std::clock();

					mtx.lock();
					OutputMemoryStream* out = new OutputMemoryStream();

					out->Write((int)Commands::UPDATE_GAME);
					out->Write(client->GetClientID());
					int size = accumulatedMessages.size();
					out->Write(size);

					for (std::pair<int, int> message : accumulatedMessages)
					{
						out->Write(message.first);
						out->Write(message.second);
					}
					
					auto _player = players->find(client->GetClientID());

					std::cout << "Client Pos: " << _player->second.posX << ", " << _player->second.posY << std::endl;

					out->Write((int)_player->second.posX);
					out->Write((int)_player->second.posY);

					Status status;
					client->GetSocket()->Send(out, status, Server_Ip, Server_Port);

					accumulatedMessages.clear();

					delete out;
					mtx.unlock();
				}

				SDL_Event _event;
				auto _player = players->find(client->GetClientID());
				float posX = _player->second.posX;
				float posY = _player->second.posY;

				float oldX = posX;
				float oldY = posY;

				int currInputX = 0, currInputY = 0;

				if (SDL_PollEvent(&_event) != 0)
				{
					switch (_event.type)
					{
					case  SDL_QUIT:
						*gameState = State::EXIT;
						break;
					case SDL_KEYDOWN:
						if (_event.key.keysym.sym == SDLK_LEFT) left = true;
						if (_event.key.keysym.sym == SDLK_RIGHT) right = true;
						if (_event.key.keysym.sym == SDLK_UP) up = true;
						if (_event.key.keysym.sym == SDLK_DOWN) down = true;
						break;
					case SDL_KEYUP:
						if (_event.key.keysym.sym == SDLK_LEFT) left = false;
						if (_event.key.keysym.sym == SDLK_RIGHT)  right = false;
						if (_event.key.keysym.sym == SDLK_UP)  up = false;
						if (_event.key.keysym.sym == SDLK_DOWN)  down = false;
						break;
					
					}
					if (left) currInputX += -1;
					if (right) currInputX += 1;
					if (up) currInputY += -1;
					if (down) currInputY += 1;
				}

				if (*gameState == State::EXIT) break;

				posX += currInputX;
				posY += currInputY;
				_player->second.currInputX = currInputX;
				_player->second.currInputY = currInputY;

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

					float lerpX = it->second.oldX + 0.01f * (it->second.posX - it->second.oldX);
					float lerpY = it->second.oldY + 0.01f * (it->second.posY - it->second.oldY);

					it->second.SetOldPlayerPos(lerpX, lerpY);

					SDL_Rect playerLocal;
					SDL_Rect r2;
					r2.x = lerpX;
					r2.y = lerpY;
					r2.w = 50;
					r2.h = 50;

					SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);

					SDL_RenderFillRect(renderer, &r2);
				}

				SDL_RenderPresent(renderer);

				if (oldX != posX || oldY != posY)
					accumulatedMessages.push_back(std::pair<int, int>(currInputX, currInputY));
			}

			SDL_DestroyWindow(window);

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

			*gameState = State::EXIT;
			break;
		}
		}
	}
}

SceneManager::SceneManager()
{
	criticalMessages = new std::map<Commands, CriticalMessages>();
	players = new std::map<int, GamePlayerInfo>();

	pong = new bool(false);

	gameState = new State( State::INIT);
	client = new GameManager();
	connected = new bool(false);
	packetId = 0;

	match = new bool(false);

	std::thread tReceive(&SceneManager::ReceiveMessages, this);
	tReceive.detach();

	std::thread tCheck(&SceneManager::CheckMessageTimeout, this);
	tCheck.detach();
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

void SceneManager::Ping(float rttKey) 
{
	//int a = 0;
	while(*gameState != State::EXIT)
	{
		std::this_thread::sleep_for(std::chrono::seconds(3));

		//std::cout << "PING: " << a << std::endl;
		//a++;
		*pong = false;
		OutputMemoryStream* out = new OutputMemoryStream();
		out->Write((int)Commands::PING_PONG);
		out->Write(client->GetClientID());
		out->Write(rttKey);

		unsigned short port;

		Status status;

		float margin = 30;
		time_t startTime;
		std::time(&startTime);

		client->GetSocket()->Send(out, status, Server_Ip, Server_Port);

		while(*pong != true) 
		{
			time_t currTime;
			std::time(&currTime);

			if(currTime - startTime > margin)
			{
				std::cout << "Disconnected!!!!!" << std::endl;
				*gameState = State::EXIT;
			}
		}

		delete out;
	}
}

void SceneManager::CheckMessageTimeout()
{
	int a = 0;
	while(*gameState != State::EXIT)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
		//std::cout << "MessageTimeout: " << a << std::endl;
		//a++;
		if (criticalMessages->size() == 0) continue;
		time_t currentTime;
		time(&currentTime);

		mtx.lock();

		Status status;
		for (auto it = criticalMessages->begin(); it != criticalMessages->end(); it++)
		{
			float time = currentTime - it->second.startTime;
			if (time > 3)
			{
				if ((int)it->first > 0 && it->first < Commands::COUNT)
				{
					std::cout << "Sending Important Message " << (int)it->first << std::endl;
					client->GetSocket()->Send(it->second.message, status, Server_Ip, Server_Port);
					it->second.startTime = currentTime;
				}
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


	std::cout << " Connecting to the server" << std::endl;

	OutputMemoryStream* out = new OutputMemoryStream();
	out->Write((int)Commands::HELLO);
	out->WriteString(client->GetName());
	out->Write(client->GetClientSalt());
		
	time_t startTime;
	time(&startTime);
	client->GetSocket()->Send(out, status, Server_Ip, Server_Port);
	SavePacketToTable(Commands::HELLO, out, startTime);

	while (!(*connected)) { std::this_thread::sleep_for(std::chrono::milliseconds(2)); }

	*gameState = State::GAME;
}

void SceneManager::ReceiveMessages()
{
	Status status;
	unsigned short _port;
	int a = 0;

	while(*gameState != State::EXIT) 
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
				//int newId;

				//in->Read(&newId);
				in->Read(&rttKey);

				//if (newId >= 0);
				OutputMemoryStream* out = new OutputMemoryStream();
				
				out->Write((int)Commands::ACK_WELCOME);
				out->Write(client->GetClientID());
				out->Write(rttKey);

				client->GetSocket()->Send(out, status, Server_Ip, Server_Port);
				MessageReceived(Commands::SALT);

				if (!*connected)
				{
					std::thread tPing(&SceneManager::Ping, this, rttKey);
					tPing.detach();
				}

				*connected = true;
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

				time_t startTime;
				time(&startTime);
				client->GetSocket()->Send(out, status, Server_Ip, Server_Port);
				SavePacketToTable(Commands::SALT, out, startTime);
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
				std::cout << id << std::endl;
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

				time_t startTime;
				time(&startTime);
				client->GetSocket()->Send(out, status, Server_Ip, Server_Port);
				SavePacketToTable(Commands::SALT, out, startTime);
				MessageReceived(Commands::HELLO);
			}
			break;
		//--------------- Connection ---------------
			
		//--------------- Disconnection ---------------
		case Commands::PING_PONG:
			{
				*pong = true;
			}
			break;
		case Commands::EXIT:
			{
			*gameState = State::END;
			}
			break;
		//--------------- Disconnection ---------------
			
		//--------------- Ingame Receives -----------
		case Commands::MATCH_FOUND:
			{
				std::cout << "New Player Joined!" << std::endl;
				float rtt;
				in->Read(&rtt);

				int id = 0;
				in->Read(&id);

				auto _player = players->find(id);
				if (_player == players->end())
				{
					GamePlayerInfo game = GamePlayerInfo(0, 0);
					players->insert(std::pair<int, GamePlayerInfo>(id, game));
				}

				OutputMemoryStream* out = new OutputMemoryStream();
				out->Write((int)Commands::ACK_MATCH_FOUND);
				out->Write((int)client->GetClientID());
				out->Write(rtt);

				client->GetSocket()->Send(out, status, Server_Ip, Server_Port);
				*match = true;
			}
			break;
		//--------------- Ingame Receives -----------
		case Commands::MATCH_FINISHED:
		{
			std::cout << "Another player has disconnected! Shutting down the match! Bye." << std::endl;
			float rtt;
			in->Read(&rtt);

			OutputMemoryStream* out = new OutputMemoryStream();
			out->Write((int)Commands::ACK_MATCH_FINISHED);
			out->Write((int)client->GetClientID());
			out->Write(rtt);
			client->GetSocket()->Send(out, status, Server_Ip, Server_Port);

			*gameState = State::END;
		}
		break;

		case Commands::UPDATE_GAME:
			{
				mtx.lock();
				int playerID;
				in->Read(&playerID);
				int posX;
				int posY;
				in->Read(&posX);
				in->Read(&posY);

				auto _player = players->find(playerID);

				_player->second.SetPlayerPos(posX, posY);

				std::cout << "PLAYER: " << std::to_string(playerID) << std::endl;
				std::cout << _player->second.posX << std::endl;
				std::cout << _player->second.posY << std::endl;
				std::cout << "_______________________________________" << std::endl;
				mtx.unlock();

			}
			break;
		case Commands::PREDICTION:
			{
				//std::cout << "PREDICTION" << std::endl;

				int _command;
				in->Read(&_command);
				if (_command == (int)Commands::CORRECT_POS)
				{

				}
				else if (_command == (int)Commands::INCORRECT_POS)
				{
					int posX;
					int posY;

					in->Read(&posX);
					in->Read(&posY);

					auto _player = players->find(client->GetClientID());
					mtx.lock();
					_player->second.SetPlayerPos(posX, posY);
					mtx.unlock();
				}

				
			}
			break;
		}

		delete in;
	}
}

SceneManager::~SceneManager()
{
	delete connected;
	delete gameState;
}

void SceneManager::Update()
{
	while (*gameState != State::EXIT)
	{
		switch (*gameState)
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