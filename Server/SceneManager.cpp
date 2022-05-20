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

	criticalMessages = new std::map<int, std::map<Commands, CriticalMessages>*>();

	if(status != Status::DONE) 
	{
		std::cout << "ERROR!!" << std::endl;
		gameState = State::END;
	}

	std::thread clientReceive(&SceneManager::ReceiveMessages, this);
	clientReceive.detach();
}

void SceneManager::MessageReceived(Commands _message, int _id, float _rttKey)
{
	auto clientPosition = criticalMessages->find(_id);
	if (clientPosition == criticalMessages->end()) return;
	mtx.lock();

	float rttMaxTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	float realRtt = rttMaxTime - _rttKey;
	game->SetClientRtt(_id, _rttKey, realRtt);

	auto mapPosition = clientPosition->second->find(_message);
	if (mapPosition != clientPosition->second->end())
	{
		delete mapPosition->second.message;
		clientPosition->second->erase(mapPosition);
	}
	mtx.unlock();

}

void SceneManager::SearchMatch(int _id, int _matchID, bool _createOrSearch)
{
	bool matchFound = false;
	game->GetClientsMap()[_id]->matchID = _matchID;
	game->GetClientsMap()[_id]->searchingForMatch = _createOrSearch;
	
	//GAME CREATED AND POLLING FOR PLAYERS
	if(!_createOrSearch) 
	{
		std::map<int, ClientData*> _clients = game->GetClientsMap();

		_clients[_id]->playerQuantity = 1;

		while (_clients[_id]->playerQuantity < 4)
		{
			_clients = game->GetClientsMap();
			for (auto it = _clients.begin(); it != _clients.end(); it++)
			{
				if (it->first == _id) continue;

				if (it->second->searchingForMatch && it->second->matchID == -1)
				{
					it->second->matchID = _clients[_id]->matchID;
					_clients[_id]->playerQuantity++;
					it->second->searchingForMatch = false;
					std::cout << "Match Found!!!!!" << std::endl;
					OutputMemoryStream* out = new OutputMemoryStream();
					out->Write((int)Commands::MATCH_FOUND);
					out->Write(_clients[_id]->matchID);
					game->SendClient(_id, out);
					delete out;
				}
			}
		}
	}
	//WAITING TO JOIN A GAME
	else 
	{
		while(!matchFound) 
		{
			if (!game->GetClientsMap()[_id]->searchingForMatch)
				matchFound = true;
		}

		OutputMemoryStream* out = new OutputMemoryStream();
		out->Write((int)Commands::MATCH_FOUND);
		out->Write(game->GetClientsMap()[_id]->matchID);

		std::cout << "Match Joined!!!!!" << std::endl;

		game->SendClient(_id, out);
		delete out;
	}
}

void SceneManager::CheckMessageTimeout()
{
	while (true)
	{
		if (criticalMessages->size() == 0) continue;
		auto currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

		mtx.lock();

		Status status;
		for (auto it = criticalMessages->begin(); it != criticalMessages->end(); it++)
		{
			bool erased = false;
			if (it->second->size() == 0) continue;
			for (auto it2 = it->second->begin(); it2 != it->second->end(); it2++)
			{
				float time = currentTime - it2->second.startTime;
				if(time > 5) 
				{
					criticalMessages->erase(it->first);
					std::cout << "Player Disconnected!!!!!" << std::endl;
					erased = true;
					break;
				}
				if (time > 1)
				{
					game->SendClient(it->first, it2->second.message);
					//it2->second.startTime = currentTime;
				}
			}
			if (erased)
				break;
		}

		mtx.unlock();
	}
}

void SceneManager::SavePacketToTable(Commands _packetId, OutputMemoryStream* out, std::time_t time, int _id)
{
	auto clientPos = criticalMessages->find(_id);

	if (clientPos == criticalMessages->end()) return;

	CriticalMessages criticalMessage = CriticalMessages(game->GetClientAddress(_id), game->GetClientPort(_id), time, out);

	auto mapPosition = clientPos->second->find(_packetId);

	// Es posa com a 0 perquè no ho tingui en compte al càlcul si encara no ha arribat
	game->SetClientRtt(_id, time, 0);

	if (mapPosition != clientPos->second->end())
	{
		delete mapPosition->second.message;
		mapPosition->second = criticalMessage;
	}
	else
	{
		clientPos->second->insert(std::pair<Commands, CriticalMessages>(_packetId, criticalMessage));
	}
}

void SceneManager::UpdateInit() 
{

}

void SceneManager::ReceiveMessages()
{
	matchID = 0;
	while (gameState != State::END)
	{
		Status status = Status::NOT_READY;

		std::pair<IpAddress, unsigned short> _client ;

		std::string ip = "127.0.0.1";
		_client.first = ip;

		InputMemoryStream* message = game->ReceiveMSG(&_client, status);

		if(status == Status::DONE) 
		{
			Commands commandNum;
			int num;

			message->Read(&num);

			commandNum = (Commands) num;

			float currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

			switch (commandNum)
			{
			case Commands::HELLO: 
				{
					OutputMemoryStream* out = new OutputMemoryStream();
					std::string name = message->ReadString();
					int salt;
					message->Read(&salt);

					int id = game->CreateClient(_client.second, _client.first, name, salt);
					out->Write(Commands::CHALLENGE);
					out->Write(currentTime);
					out->Write(id);
					out->Write(game->GetServerSalt(id));

					criticalMessages->insert(std::pair<int, std::map<Commands, CriticalMessages>*>(id, new std::map<Commands, CriticalMessages>()));

					auto currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

					game->SendClient(id, out);
					SavePacketToTable(Commands::CHALLENGE, out, currentTime, id);
				}
				break;
			case Commands::SALT:
				{
					float rttKey;
					int id, salt;
					message->Read(&rttKey);
					message->Read(&id);
					message->Read(&salt);

					if (game->GetClientsMap()[id]->GetTries() >= MAX_TRIES)
					{
						game->GetClientsMap().erase(id);
						continue;
					}

					OutputMemoryStream* out = new OutputMemoryStream();

					int _result = game->GetServerSalt(id) & game->GetClientSalt(id);
					
					std::cout << "Server SALT: " << game->GetServerSalt(id) << ", Client SALT: " << game->GetClientSalt(id) << ", Result: " << _result << std::endl;

					MessageReceived(Commands::SALT, id, rttKey);
					MessageReceived(Commands::CHALLENGE, id, rttKey);

					if(salt == _result)
					{
						out->Write((int) Commands::WELCOME);
						SavePacketToTable(Commands::WELCOME, out, currentTime, id);
					}
					else 
					{
						out->Write((int) Commands::SALT);
						SavePacketToTable(Commands::SALT, out, currentTime, id);
						game->GetClientsMap()[id]->AddTry();
					}
					out->Write(currentTime);


					game->SendClient(id, out);
				}
				break;
			case Commands::ACK_WELCOME:
				{
					std::cout << "client connected" << std::endl;
					float rttKey;
					int id;
					message->Read(&rttKey);
					message->Read(&id);
					MessageReceived(Commands::WELCOME, id, rttKey);
				}
				break;
			case Commands::PING_PONG:
				{
					int id;
					message->Read(&id);
					OutputMemoryStream* out = new OutputMemoryStream();
					
					out->Write((int)Commands::PING_PONG);
					out->Write(currentTime);

					auto currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

					game->SendClient(id, out);
					SavePacketToTable(Commands::PING_PONG, out, currentTime, id);
				}
				break;
			case Commands::SEARCH_MATCH:
				{
					int id;
					message->Read(&id);
					bool createOrSearch;
					message->Read(&createOrSearch);
					matchID++;

					if (createOrSearch)
					{
						std::thread tSearch(&SceneManager::SearchMatch, this, id, -1, createOrSearch);
						tSearch.detach();
					}
					else
					{
						std::thread tCreate(&SceneManager::SearchMatch, this, id, matchID, createOrSearch);
						tCreate.detach();
					}
				}
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
	for (auto it = criticalMessages->begin(); it != criticalMessages->end(); it++)
	{
		delete it->second;
	}
	delete criticalMessages;
}

void SceneManager::Update()
{
	std::thread tCheck(&SceneManager::CheckMessageTimeout, this);
	tCheck.detach();

	while (gameState != State::END)
	{
		ReceiveMessages();
	}
}