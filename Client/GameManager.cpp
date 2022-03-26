#include "GameManager.h"
#include "SceneManager.h"
#include <thread>
#include "../res/TcpSocket.h"
#include "../res/Selector.h"

void GameManager::ClientControl(TcpSocket* serverSock)
{
	Selector selector;

	TcpListener listener;
	Status status = listener.Listen(localPort);
	if (status != Status::DONE)
		return;

	selector.Add(&listener);
	selector.Add(serverSock);

	while (true)
	{
		if (selector.Wait())
		{
			//Accept peers
			if (selector.IsReady(&listener))
			{
				AcceptConnections(&selector, &listener);
			}
			//Connect peers
			else if (serverSock != nullptr && selector.IsReady(serverSock))
			{
				InputMemoryStream* in;
				in = serverSock->Receive(status);
				if (status != Status::DONE)
				{
					delete in;
					selector.Remove(serverSock);
					continue;
				}
				OutputMemoryStream* out = new OutputMemoryStream();

				int instruction;
				in->Read(&instruction);

				//----------------------------// CREATE GAME //----------------------------//

				//Adptar el connect game que hagi fet el Mateu
				/*if (instruction == (int)Commands::CREATE_GAME)
				{

				}*/

				//----------------------------// SEARCH GAME //----------------------------//

				if (instruction == (int)Commands::GAME_LIST)
				{
					ListCurrentGames(in);
				}

				//--------------------------// JOIN GAME LOGIC //--------------------------//
				// JOIN GAME
				else if (instruction == (int)Commands::PROTECTED)
				{
					SendPassword(out);
				}
				else if (instruction == (int)Commands::NOT_PROTECTED)
				{
					//Check ready

				}
				else if (instruction == (int)Commands::INCORRECT_ID)
				{
					JoinGame(out);
				}
				// PWD CHECK
				else if(instruction == (int)Commands::CORRECT_PWD)
				{
					//Check ready

				}
				else if (instruction == (int)Commands::INCORRECT_PWD)
				{
					SendPassword(out);
				}
				//------------------------// END JOIN GAME LOGIC //------------------------//
				else if (instruction == (int)Commands::PLAYER_LIST)
				{
					ConnectP2P(&selector, serverSock, in);
					delete in;
					delete out;

					continue;
				}

				serverSock->Send(out, status);
				delete in;
				delete out;
			}
			//Peer receives
			else
			{
				for (size_t i = 0; i < socks->size(); i++)
				{
					if (selector.IsReady(socks->at(i)))
					{
						InputMemoryStream* in;
						in = socks->at(i)->Receive(status);
						if (status != Status::DONE)
						{
							delete in;
							selector.Remove(socks->at(i));
							continue;
						}
						OutputMemoryStream* out = new OutputMemoryStream();

						int instruction;
						in->Read(&instruction);

						//-----------------------------// GAMELOOP //------------------------------//

						//Turn system
						if (instruction == (int)Commands::ORGAN_QUANTITY)
						{
							int playerID, organQuantity;

							in->Read(&playerID);

							in->Read(&organQuantity);

							if (playerTurnOrder.size() < socks->size() + 1)
							{
								playerTurnOrder.push_back(Pair_Organ_Player(playerID, organQuantity));
							}
							else
							{
								for (size_t i = 0; i < playerTurnOrder.size(); i++)
								{
									if (playerTurnOrder[i].playerID == playerID)
									{
										playerTurnOrder[i].numOrgans = organQuantity;
										break;
									}
								}
							}

							std::sort(playerTurnOrder.begin(), playerTurnOrder.end(), ComparePlayers);
						}
						//Receive turn
						else if (instruction == (int)Commands::UPDATE_TURN)
						{
							int _currentTurn;
							in->Read(&_currentTurn);

							if (_currentTurn >= playerTurnOrder.size())
							{
								_currentTurn = 0;
								*endRound = true;
							}

							*currentTurn = _currentTurn;
						}
						else if (instruction == (int)Commands::PLAYER_READY)
						{
							bool isReady;
							in->Read(&isReady);

							if (isReady)
							{
								int* newPlayersReady = new int(*playersReady + 1);
								delete playersReady;
								playersReady = newPlayersReady;
							}
						}

						//---------------------------// END GAMELOOP //----------------------------//

						socks->at(i)->Send(out, status);
						delete in;
						delete out;
					}
				}
			}
		}
	}

	selector.Clear();
}

void GameManager::CalculateOrganQuantity()
{
	int organQuantity = 0;
	for (size_t i = 0; i < player->hand.hand.size(); i++)
	{
		if (player->hand.hand[i]->cardType == Card::CardType::ORGAN)
			organQuantity++;
	}

	if (playerTurnOrder.size() < socks->size() + 1) 
	{
		mtx.lock();
		playerTurnOrder.push_back(Pair_Organ_Player(player->id, organQuantity));
		mtx.unlock();
	}
	else
	{
		for (size_t i = 0; i < playerTurnOrder.size(); i++)
		{
			if (playerTurnOrder[i].playerID == player->id) 
			{
				playerTurnOrder[i].numOrgans = organQuantity;
				break;
			}
		}
	}

	//CheckArray();
	
	/*if (playerNum == 1)
		player1Organs = organQuantity;
	else if (playerNum == 2)
		player2Organs = organQuantity;
	else if (playerNum == 3)
		player3Organs = organQuantity;
	else if (playerNum == 4)
		player4Organs = organQuantity;*/

	OutputMemoryStream* out = new OutputMemoryStream();
	//instruction 0: receive the organ quantity to receive the turn
	out->Write((int)Commands::ORGAN_QUANTITY);
	out->Write(player->id);
	out->Write(organQuantity);

	std::cout << organQuantity << std::endl;

	for (int i = 0; i < socks->size(); i++)
	{
		Status status;
		socks->at(i)->Send(out, status);
	}

	delete(out);
}

void GameManager::UpdateTurn(bool plus)
{
	OutputMemoryStream* out = new OutputMemoryStream();
	
	if(plus) 
	{
		int* value = new int(*currentTurn + 1);
		delete currentTurn;
		currentTurn = value;
	}

	//instruction 1: send your turn to another player
	out->Write((int)Commands::UPDATE_TURN);
	out->Write(*currentTurn);

	Status status;

	for (int i = 0; i < socks->size(); i++)
	{
		socks->at(i)->Send(out, status);
	}

	delete out;
}

GameManager::~GameManager()
{
	for (int i = 0; i < socks->size(); i++)
		delete socks->at(i);
	delete socks;

	delete player;
	delete deck;
	delete table;

	delete endRound;
	delete currentTurn;
}

bool GameManager::Update()
{
	if (*currentTurn == playerTurnOrder.size() || playerTurnOrder[*currentTurn].playerID != player->id)
		return *endRound;

	for (size_t i = 0; i < playerTurnOrder.size(); i++)
		std::cout << "turn player: " << playerTurnOrder[i].playerID << std::endl;

	player->ReceiveCards(MAX_CARDS - player->hand.hand.size(), deck);

	std::cout << "" << std::endl;
	std::cout << "___________MENU___________" << std::endl;
	std::cout << "1. Place Organ" << std::endl;
	std::cout << "2. Infect Other Organ" << std::endl;
	std::cout << "3. Vaccine Organ" << std::endl;
	std::cout << "4. Discard card" << std::endl;
	std::cout << "5. Deploy threatment card" << std::endl;
	std::cout << "___________MENU___________" << std::endl;
	std::cout << "" << std::endl;

	std::cout << "___________TABLE___________" << std::endl;
	table->ShowTable();
	std::cout << "___________TABLE___________" << std::endl;
	std::cout << "" << std::endl;

	Commands option;
	int tmpOption;
	std::cin >> tmpOption;
	tmpOption += 8;
	option = (Commands)tmpOption;

	int card;
	
	switch (option)
	{
	case Commands::PLACE_ORGAN:
		player->hand.ListCards();
		std::cout << "Choose a card: ";
		
		std::cin >> card;

		player->PlaceCard(card, Card::CardType::ORGAN, table, deck);

		std::cout << "Organ Placed!" << std::endl;
		break;
	case Commands::PLACE_INFECTION:
		break;
	case Commands::PLACE_MEDICINE:
		break;
	case Commands::PLACE_TREATMENT:
		break;
	case Commands::DISCARD_CARD:
		player->hand.ListCards();
		std::cout << "Choose a card: ";

		std::cin >> card;

		player->hand.hand.erase(player->hand.hand.begin() + card);
		std::cout << "Card Removed!" << std::endl;
		break;

	default:;
	}

	std::cout << "waiting other players" << std::endl;

	UpdateTurn(true);

	//Mostrar missatges de tota la ronda (events)

	//__________________________________

	std::cout << "Waiting For your turn" << std::endl;

	return *endRound;

	/*std::string msg;
	std::cin >> msg;

	OutputMemoryStream* out = new OutputMemoryStream();
	out->WriteString(msg);
	std::cout << _socks->size() << std::endl;
	for (int i = 0; i < _socks->size(); i++) {
		Status status;
		_socks->at(i)->Send(out, status);
		std::cout << (int)status << std::endl;
	}
	if (msg == "e") {
		end = true;
		for (int i = 0; i < _socks->size(); i++) {
			_socks->at(i)->Disconnect();
			delete _socks->at(i);
		}
		_socks->clear();
	}
	delete out;*/
}

void GameManager::Start()
{
	player->ReceiveCards(3, deck);
	player->hand.ListCards();
	CalculateOrganQuantity();
	UpdateTurn(false);
}

void GameManager::SendReady()
{
	OutputMemoryStream* out = new OutputMemoryStream();
	out->Write((int)Commands::PLAYER_READY);
	out->Write(true);

	Status status;

	for (size_t i = 0; i < socks->size(); i++)
	{
		socks->at(i)->Send(out, status);
	}

	delete out;
}

void GameManager::SendPassword(OutputMemoryStream* out)
{
	int gameID = *currentGameID;
	out->Write(gameID);

	std::string password;
	std::cin >> password;

	out->WriteString(password);

}

void GameManager::SetReady()
{
	ready = true;

	int* value = new int(*playersReady + 1);
	delete playersReady;
	playersReady = value;

	SendReady();
}

void GameManager::ReceiveMessages(TcpSocket* _sock, int* _sceneState)
{
	while (*_sceneState != (int)SceneManager::Scene::GAMEOVER) {
		Status status;
		InputMemoryStream in1 = *_sock->Receive(status);

		if (status != Status::DONE)
			return;

		mtx.lock();
		int instruction = 0;
		in1.Read(&instruction);
		mtx.unlock();

		//Turn system
		if (instruction == (int)Commands::ORGAN_QUANTITY)
		{
			mtx.lock();
			int playerID, organQuantity;

			in1.Read(&playerID);

			in1.Read(&organQuantity);
			mtx.unlock();

			if (playerTurnOrder.size() < socks->size() + 1) 
			{
				mtx.lock();
				playerTurnOrder.push_back(Pair_Organ_Player(playerID, organQuantity));
				mtx.unlock();
			}
			else
			{
				for (size_t i = 0; i < playerTurnOrder.size(); i++)
				{
					if (playerTurnOrder[i].playerID == playerID)
					{
						playerTurnOrder[i].numOrgans = organQuantity;
						break;
					}
				}
			}

			std::sort(playerTurnOrder.begin(), playerTurnOrder.end(), ComparePlayers);
			//CheckArray();
		}
		//Receive turn
		else if (instruction == (int)Commands::UPDATE_TURN)
		{
			mtx.lock();
			int turn;
			in1.Read(&turn);
			mtx.unlock();
			if (turn >= playerTurnOrder.size())
			{
				turn = 0;
				*endRound = true;
			}
				
			*currentTurn = turn;
		}
		//Receive a card from another player
		else if (instruction == 3)
		{

		}
		else if (instruction == (int)Commands::PLAYER_READY)
		{
			mtx.lock();
			bool isReady;
			in1.Read(&isReady);
			mtx.unlock();
			if (isReady)
			{
				int* value = new int(*playersReady + 1);
				delete playersReady;
				playersReady = value;
			}
		}

		//std::string msg = in->ReadString();
		//std::cout << msg << std::endl;
		/*if (msg == "e" || status != Status::DONE) {
			std::cout << "Socket with ip: " << _sock->GetRemoteAddress() << " and port: " << _sock->GetLocalPort() << " was disconnected" << std::endl;
			for (auto it = _socks->begin(); it != _socks->end(); it++) {
				if (*it == _sock) {
					_socks->erase(it);
					delete _sock;
					delete in;
					return;
				}
			}
		}*/
	}
}

void GameManager::AcceptConnections(Selector* selector, TcpListener* listener)
{
	TcpSocket* sock = new TcpSocket();
	Status status = listener->Accept(*sock);
	if (status != Status::DONE) {
		delete sock;
		return;
	}
	socks->push_back(sock);
	std::cout << "Connected with ip: " << sock->GetRemoteAddress() << " and port: " << sock->GetLocalPort() << std::endl;

	table->table.push_back(std::vector<Card*>());

	selector->Add(sock);
}

void GameManager::CreateGame(TcpSocket* serverSock)
{
	OutputMemoryStream* out = new OutputMemoryStream();
	//GAME NAME
	std::cout << "Set a Name for this Game" << std::endl;

	std::string gameName;
	std::cin >> gameName;
	out->WriteString(gameName);

	//GAME MAX PLAYERS
	std::cout << "Set the Maximum of Players for this Game" << std::endl;

	int maxPlayers;
	char num;
	
	std::cin >> num;
	maxPlayers = num - '0';

	while (maxPlayers < 2 && maxPlayers > 4)
	{
		std::cout << "Error setting the max number of players. Type again" << std::endl;
		std::cin >> num;
		maxPlayers = num - '0';
	} 
	out->Write(maxPlayers);

	//GAME PASSWORD
	std::string password = "Set a Password for this Game (type '-' to leave it empty)";
	std::cin >> password;
	out->WriteString(password);

	Status status;
	serverSock->Send(out, status);

	delete out;
}

void GameManager::ListCurrentGames(InputMemoryStream* in)
{
	//Get Num of games
	int numOfGames;
	in->Read(&numOfGames);
	
	for (int i = 0; i < numOfGames; i++)
	{
		int gameID;
		in->Read(&gameID);

		std::string gameName = in->ReadString();

		int gameSize;
		in->Read(&gameSize);

		int numOfPlayers;
		in->Read(&numOfPlayers);
		std::cout << "Game ID: " << gameID << ", Game Name: " + gameName << ", Max players: " << gameSize << ", Players connected: " << numOfPlayers << std::endl;
	}
}

void GameManager::JoinGame(OutputMemoryStream* out)
{
	//Choose game
	std::cout << "Type server ID" << std::endl;
	char tmpOption;
	std::cin >> tmpOption;
	int serverIdx = tmpOption - '0';

	*currentGameID = serverIdx;

	out->Write(serverIdx);
}

void GameManager::ConnectP2P(Selector* selector, TcpSocket* serverSock, InputMemoryStream* in)
{
	Status status;

	int playerNum;
	in->Read(&playerNum);

	selector->Remove(serverSock);
	serverSock->Disconnect();

	table->table.push_back(std::vector<Card*>());

	for (size_t i = 0; i < playerNum; i++)
	{
		TcpSocket* peer = new TcpSocket();

		std::string peerIp = in->ReadString();
		int peerPort;
		in->Read(&peerPort);

		status = peer->Connect(peerIp, peerPort);
		if (status != Status::DONE)
		{
			delete peer;
			continue;
		}
		std::cout << "Connected with ip: " << peerIp << " and port: " << peerPort << std::endl;
		socks->push_back(peer);

		selector->Add(peer);

		table->table.push_back(std::vector<Card*>());
	}

	player->id = socks->size();
}

