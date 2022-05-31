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
	gameState = new State (State::INIT);
	
	std::thread exitThread(&SceneManager::ExitThread, this);
	exitThread.detach();

	game = new GameManager();
	Status status;
	status = game->BindSocket();

	if(status != Status::DONE) 
	{
		std::cout << "ERROR!!" << std::endl;
		*gameState = State::END;
	}

	criticalMessages = new std::map<int, std::map<Commands, CriticalMessages>*>();
	rooms = new std::map<int, std::vector<std::pair<int, ClientData*>>>();

	searchingPlayers = new std::vector<std::pair<int, ClientData*>>();

	std::thread roomsThread(&SceneManager::CheckRooms, this);
	roomsThread.detach();

	std::thread matchMakingThread(&SceneManager::MatchMaking, this);
	matchMakingThread.detach();
}

void SceneManager::MessageReceived(Commands _message, int _id, float _rttKey)
{
	auto clientPosition = criticalMessages->find(_id);
	if (clientPosition == criticalMessages->end()) return;

	float rttMaxTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	float realRtt = rttMaxTime - _rttKey;
	game->SetClientRtt(_id, _rttKey, realRtt);

	auto mapPosition = clientPosition->second->find(_message);
	if (mapPosition != clientPosition->second->end())
	{
		mtx.lock();
		delete mapPosition->second.message;
		clientPosition->second->erase(mapPosition);
		mtx.unlock();
	}
}

void SceneManager::UpdateGameInfo(int _gameID, int hostID) 
{
	std::map<int, ClientData*>* _clients = game->GetClientsMap();
	int a = 0;

	while (*gameState != State::END)
	{
		if (game->GetConnectedClient(hostID)->disconnected) return;
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		//std::cout << "UPDATEGAME: " << a << std::endl;
		//a++;

		for (auto it = _clients->begin(); it != _clients->end(); it++)
		{
			if (it->second->matchID == _gameID && !it->second->disconnected)
			{
				//sending their info to the rest of clients
				for (auto it2 = _clients->begin(); it2 != _clients->end(); it2++)
				{
					if (it2->first == it->first) continue;
					if (it2->second->matchID == _gameID && !it2->second->disconnected)
					{
						OutputMemoryStream* out = new OutputMemoryStream();
						out->Write((int)Commands::UPDATE_GAME);
						out->Write(it->first);
						out->Write(it->second->GetXPos());
						out->Write(it->second->GetYPos());
						it->second->UpdatePosition();
						game->SendClient(it2->first, out);
						delete out;
					}
				}
			}
		}
	}
}

void SceneManager::CheckRooms()
{
	OutputMemoryStream* out;
	while (*gameState != State::END)
	{
		if (rooms->size() == 0)
		{
			std::this_thread::sleep_for(std::chrono::seconds(2));
			continue;
		}

		bool breakLoop = false;

		for (auto roomIt = rooms->begin(); roomIt != rooms->end(); roomIt++)
		{
			for (int i = 0; i < roomIt->second.size(); i++)
			{
				if (roomIt->second[i].second->disconnected)
				{
					OutputMemoryStream* out2 = new OutputMemoryStream();
					out2->Write((int)Commands::MATCH_FINISHED);
					if (i == 0)
					{
						SavePacketToTable(Commands::MATCH_FINISHED, out2,
							std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()), roomIt->second[1].first);
						game->SendClient(roomIt->second[1].first, out2);
					}
					else
					{
						SavePacketToTable(Commands::MATCH_FINISHED, out2,
							std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()), roomIt->second[0].first);
						game->SendClient(roomIt->second[0].first, out2);
					}
					rooms->erase(roomIt);

					breakLoop = true;

					break;
				}
				for (size_t j = 0; j < roomIt->second.size(); j++)
				{
					if (j == i) continue;

					bool sendInfo = false;

					for (int l = 0 ; l < roomIt->second[i].second->positions.size() ; l++)
					{
						mtx.lock();
						roomIt->second[i].second->UpdatePosition();
						mtx.unlock();
						sendInfo = true;
					}
					out = new OutputMemoryStream();
					out->Write((int)Commands::UPDATE_GAME);
					out->Write(roomIt->second[i].first);
					out->Write(roomIt->second[i].second->GetXPos());
					out->Write(roomIt->second[i].second->GetYPos());
					//std::cout << "Player: " << roomIt->second[j].first << " X: " << roomIt->second[i].second->GetXPos() << " Y: " << roomIt->second[i].second->GetYPos() << std::endl;
					if(sendInfo)
					game->SendClient(roomIt->second[j].first, out);
				}
			}
			if (breakLoop) break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

void SceneManager::MatchMaking()
{
	OutputMemoryStream* out;
	while (*gameState != State::END)
	{
		if (searchingPlayers->empty())
		{
			std::this_thread::sleep_for(std::chrono::seconds(2));
			continue;
		}

		bool breakLoop = false;

		for (size_t i = 0; i < searchingPlayers->size(); i++)
		{
			for (size_t j = 0; j < searchingPlayers->size(); j++)
			{
				if (i == j) continue;
				if (searchingPlayers->at(j).second->searchingForMatch && abs(searchingPlayers->at(i).second->GetName()[0] - searchingPlayers->at(j).second->GetName()[0]) < 10)
				{
					out = new OutputMemoryStream();
					std::pair<int, std::vector<std::pair<int, ClientData*>>> _room(matchID, std::vector< std::pair<int, ClientData*>>());
					_room.second.push_back(std::pair<int, ClientData*>(searchingPlayers->at(i).second->GetId(), searchingPlayers->at(i).second));
					_room.second.push_back(std::pair<int, ClientData*>(searchingPlayers->at(j).second->GetId(), searchingPlayers->at(j).second));

					matchID++;
					rooms->insert(_room);
					
					out->Write((int)Commands::MATCH_FOUND);
					float rtt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
					out->Write(rtt);
					//SavePacketToTable(Commands::MATCH_FOUND, out,
						//std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()), searchingPlayers->at(i).second->GetId());
					//SavePacketToTable(Commands::MATCH_FOUND, out,
						//std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()), searchingPlayers->at(j).second->GetId());

					game->SendClient(searchingPlayers->at(i).second->GetId(), out);
					game->SendClient(searchingPlayers->at(j).second->GetId(), out);

					delete out;

					if (i < j)
					{
							searchingPlayers->at(i).second->searchingForMatch = false;
							searchingPlayers->at(j).second->searchingForMatch = false;
					}
					else
					{
							searchingPlayers->at(j).second->searchingForMatch = false;
							searchingPlayers->at(i).second->searchingForMatch = false;
					}

					breakLoop = true;
					break;
				}
				else if(!searchingPlayers->at(j).second->searchingForMatch)
				{
					if (i < j)
					{
							searchingPlayers->erase(searchingPlayers->begin() + i);
							searchingPlayers->erase(searchingPlayers->begin() + j - 1);
					}
					else
					{
							searchingPlayers->erase(searchingPlayers->begin() + j);
							searchingPlayers->erase(searchingPlayers->begin() + i - 1);
					}
				}
			}
			if (breakLoop) break;
		}
	}
}

void SceneManager::SearchMatch(int _id, int _matchID, bool _createOrSearch)
{
	bool matchFound = false;
	game->GetConnectedClient(_id)->matchID = _matchID;
	game->GetConnectedClient(_id)->searchingForMatch = _createOrSearch;
	
	//GAME CREATED AND POLLING FOR PLAYERS
	if(!_createOrSearch) 
	{
		std::map<int, ClientData*>* _clients = game->GetClientsMap();

		_clients->at(_id)->playerQuantity = 1;

		while (*gameState != State::END)
		{
			if (game->GetConnectedClient(_id)->disconnected) return;
			//std::cout << "MATCHFIND: " << a << std::endl;
			//a++;
			//_clients = game->GetClientsMap();
			for (auto it = _clients->begin(); it != _clients->end(); it++)
			{
				if (it->first == _id) continue;

				ClientData* _clientInfo = _clients->at(_id);

				if (it->second->searchingForMatch && it->second->matchID == -1 && !it->second->disconnected)
				{
					if (abs(_clientInfo->GetName()[0] - it->second->GetName()[0]) < 10)
					{
						it->second->matchID = _clientInfo->matchID;
						_clientInfo->playerQuantity++;
						it->second->searchingForMatch = false;
						std::cout << "Player Found!!!!!" << std::endl;
						OutputMemoryStream* out = new OutputMemoryStream();
						out->Write((int)Commands::MATCH_FOUND);
						out->Write(_clientInfo->matchID);
						game->SendClient(_id, out);

						if (!matchFound)
						{
							matchFound = true;
							return;
						}

						delete out;
					}
				}
			}
		}
	}
	//WAITING TO JOIN A GAME
	else 
	{
		ClientData* _client = game->GetClientsMap()->at(_id);
		if (_client == nullptr) return;
		while(!matchFound) 
		{
			if (_client->disconnected) return;
			if (!_client->searchingForMatch)
				matchFound = true;
		}

		OutputMemoryStream* out = new OutputMemoryStream();
		out->Write((int)Commands::MATCH_FOUND);
		out->Write(_client->matchID);

		std::cout << "Match Joined!!!!!" << std::endl;

		game->SendClient(_id, out);
		delete out;
	}
}

void SceneManager::DisconnectClient(int _id)
{
	game->DisconnectClient(_id);
	auto _client = criticalMessages->find(_id);
	if (_client != criticalMessages->end())
	{
		delete _client->second;
		criticalMessages->erase(_id);
		std::cout << "Player Disconnected!!!!!" << std::endl;
	}

}

void SceneManager::ExitThread()
{
	std::string _exit = "";
	int a = 0;
	while (*gameState != State::END)
	{
		//std::cout << "EXIT: " << a << std::endl;
		//a++;
		std::cin >> _exit;
		if (_exit == "exit" || _exit == "EXIT")
		{
			mtx.lock();
			*gameState = State::END;
			OutputMemoryStream* out = new OutputMemoryStream();
			out->Write((int)Commands::EXIT);

			Status status;
			game->SendAll(out);
			delete out;

			mtx.unlock();
			*gameState = State::END;
		}
	}
}

void SceneManager::CheckMessageTimeout()
{
	int a = 0;
	while (*gameState != State::END)
	{
		//std::cout << "TIMEOUTMSG: " << a << std::endl;
		//a++;
		std::this_thread::sleep_for(std::chrono::seconds(1));
		if (criticalMessages->size() == 0) {
			continue;
		}
	
		//mtx.lock();
		Status status;
		
		for (auto it = criticalMessages->begin(); it != criticalMessages->end(); it++)
		{	
			bool breakLoop = false;
			if (it->second->size() == 0)
				continue;
			for (auto it2 = it->second->begin(); it2 != it->second->end(); it2++)
			{
				//std::cout << it2->second.tries << std::endl;
				if(it2->first == Commands::PING_PONG && it2->second.tries > 10)
				{
					DisconnectClient(it->first);
					breakLoop = true;
					break;
				}
				else if(!game->GetClient(it->first)->disconnected)
				{
					std::cout << "Sending Important Message " <<  (int)it2->first << std::endl;
					game->SendClient(it->first, it2->second.message);
					it2->second.tries++;
				}
			}
			if (breakLoop) break;
		}
		//mtx.unlock();

	}
}

void SceneManager::SavePacketToTable(Commands _packetId, OutputMemoryStream* out, std::time_t time, int _id)
{
	auto clientPos = criticalMessages->find(_id);

	if (clientPos == criticalMessages->end()) return;
	mtx.lock();

	CriticalMessages criticalMessage = CriticalMessages(game->GetClient(_id)->GetAddress(), game->GetClient(_id)->GetPort(), time, out);

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

	mtx.unlock();
}

void SceneManager::UpdateInit() 
{

}

void SceneManager::ReceiveMessages()
{
	Status status = Status::NOT_READY;
	std::string ip = "127.0.0.1";

	std::pair<IpAddress, unsigned short> _client = std::pair<IpAddress, unsigned short>(ip, 0);

	mtx.lock();

	InputMemoryStream* message = game->ReceiveMSG(&_client, status);

	mtx.unlock();

	if(status == Status::DONE) 
	{
		Commands commandNum;
		int num;

		message->Read(&num);

		commandNum = (Commands) num;

		int id;
		float currentTime;
		//Check if Client is connected or connecting
		if (commandNum != Commands::HELLO)
		{
			message->Read(&id);
			if (game->GetConnectedClient(id) == nullptr && game->GetConnectingClient(id) == nullptr)
			{
				return;
			}
		}
		/*else 
		{
			std::pair<int, ClientData*> _client;

			if (_client.first >= 0)
			{
				currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

				OutputMemoryStream* out = new OutputMemoryStream();
				out->Write((int)Commands::WELCOME);
				out->Write(_client.first);
				SavePacketToTable(Commands::WELCOME, out, currentTime, _client.first);
				return;

			}
		}*/


		switch (commandNum)
		{
		//---------------Connection---------------
		case Commands::HELLO:
			{
				OutputMemoryStream* out = new OutputMemoryStream();
				std::string name = message->ReadString();
				int salt;
				message->Read(&salt);

				id = game->CreateClient(_client.second, _client.first, name, salt);

				currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

				out->Write(Commands::CHALLENGE);
				out->Write(currentTime);
				out->Write(id);
				out->Write(game->GetConnectingClient(id)->GetServerSalt());

				criticalMessages->insert(std::pair<int, std::map<Commands, CriticalMessages>*>(id, new std::map<Commands, CriticalMessages>()));

				game->SendClient(id, out);
				SavePacketToTable(Commands::CHALLENGE, out, currentTime, id);
			}
			break;
		case Commands::SALT:
			{
				float rttKey;
				int salt;
				message->Read(&rttKey);
				message->Read(&salt);

				ClientData* _client = game->GetClient(id);
				if (_client == nullptr) return;

				if (_client->GetTries() >= MAX_TRIES)
				{
					game->DisconnectClient(id);
					return;
				}

				OutputMemoryStream* out = new OutputMemoryStream();

				int _result = _client->GetServerSalt() & _client->GetClientSalt();

				std::cout << "Server SALT: " << _client->GetServerSalt() << ", Client SALT: " << _client->GetClientSalt() << ", Result: " << _result << std::endl;

				MessageReceived(Commands::SALT, id, rttKey);
				MessageReceived(Commands::CHALLENGE, id, rttKey);

				if (salt == _result)
				{
					currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
					out->Write((int)Commands::WELCOME);
					out->Write(-1);
					SavePacketToTable(Commands::WELCOME, out, currentTime, id);
				}
				else
				{
					currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
					out->Write((int)Commands::SALT);
					SavePacketToTable(Commands::SALT, out, currentTime, id);
					game->GetClientsMap()->at(id)->AddTry();
				}
				out->Write(currentTime);

				game->SendClient(id, out);
		}
			break;
		case Commands::ACK_WELCOME:
			{
				std::cout << "client connected" << std::endl;
				float rttKey;

				message->Read(&rttKey);
				MessageReceived(Commands::WELCOME, id, rttKey);

				game->ClientConnected(id);
		}
			break;
		//--------------- Connection ---------------

		//--------------- Disconnection ---------------
		case Commands::PING_PONG:
			{
				float rttKey;
				message->Read(&rttKey);

				MessageReceived(Commands::PING_PONG, id, rttKey);

				OutputMemoryStream* out = new OutputMemoryStream();
				currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
					
				out->Write((int)Commands::PING_PONG);
				out->Write(currentTime);

				game->SendClient(id, out);
				SavePacketToTable(Commands::PING_PONG, out, currentTime, id);
			}
			break;
		case Commands::EXIT:
			{
				DisconnectClient(id);
			}
			break;
		//--------------- Disconnection ---------------

		//--------------- Ingame Receives -----------
		case Commands::SEARCH_MATCH:
			{
				bool createOrSearch = false;
				game->GetConnectedClient(id)->searchingForMatch = true;
				searchingPlayers->push_back(std::pair<int, ClientData*>(id, game->GetConnectedClient(id)));


				/*matchID++;

				std::map<int, ClientData*>* _clients = game->GetClientsMap();
				
				if(_clients->size() == 1) createOrSearch = true;

				for (auto it = _clients->begin(); it != _clients->end(); it++)
				{
					if (it->first == id) continue;
					if (it->second->disconnected) continue;

					if (!it->second->searchingForMatch)
					{
						if(abs(_clients->at(id)->GetName()[0] - it->second->GetName()[0] ) < 10)
							createOrSearch = true;
					}
				}

				if (createOrSearch)
				{
					std::cout << "SEARCHING A GAME FOR PLAYER " << id;
					std::thread tSearch(&SceneManager::SearchMatch, this, id, -1, createOrSearch);
					tSearch.detach();
				}
				else
				{
					std::cout << "CREATING A GAME FOR PLAYER " << id;
					std::thread tCreate(&SceneManager::SearchMatch, this, id, matchID, createOrSearch);
					tCreate.detach();
				}*/
			}
			break;
		//--------------- Ingame Receives -----------
		case Commands::UPDATE_GAME:
			{
				int quantity = 0;
				int empty = 0;

				message->Read(&quantity);
				message->Read(&empty);


				int serverXpos = game->GetConnectedClient(id)->GetXPos();
				int serverYpos = game->GetConnectedClient(id)->GetYPos();

				for (size_t i = 0; i < quantity; i++)
				{
					int posX = 0;
					int posY = 0;

					message->Read(&posX);
					message->Read(&posY);

					serverXpos += posX;
					serverYpos += posY;

					mtx.lock();
					game->GetConnectedClient(id)->AcumulatePosition(posX, posY);
					mtx.unlock();
				}

				int clientPosX;
				int clientPosY;

				message->Read(&clientPosX);
				message->Read(&clientPosY);

				if(clientPosX != serverXpos || clientPosY != serverYpos) 
				{
					OutputMemoryStream* out = new OutputMemoryStream();
					out->Write((int)Commands::PREDICTION);
					out->Write(serverXpos);
					out->Write(serverYpos);

					game->SendClient(id, out);
				}
			}
			break;
		case Commands::ACK_MATCH_FOUND:
		{
			float rttKey;
			message->Read(&rttKey);
			MessageReceived(Commands::MATCH_FOUND, id, rttKey);
		}
		break;
		case Commands::ACK_MATCH_FINISHED:
		{
			float rttKey;
			message->Read(&rttKey);
			MessageReceived(Commands::MATCH_FINISHED, id, rttKey);
		}
		break;
		}
	}
	delete message;
}

SceneManager::~SceneManager()
{
	delete game;
	delete gameState;

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
	while (*gameState != State::END)
	{
		ReceiveMessages();
	}
}