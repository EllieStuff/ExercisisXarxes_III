#include "GameManager.h"
#include "SceneManager.h"
#include <thread>
#include "../res/TcpSocket.h"

void GameManager::CalculateOrganQuantity()
{
	/*mtx.lock();*/
	int organQuantity = 0;
	for (size_t i = 0; i < player->hand.hand.size(); i++)
	{
		if (player->hand.hand[i]->cardType == Card::CardType::ORGAN)
			organQuantity++;
	}

	if (playerTurnOrder.size() < socks->size() + 1) 
		playerTurnOrder.push_back(Pair_Organ_Player(player->id, organQuantity));
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

	//mtx.unlock();
}

void GameManager::UpdateTurn()
{
	OutputMemoryStream* out = new OutputMemoryStream();

	//instruction 1: send your turn to another player
	out->Write((int)Commands::UPDATE_TURN);
	out->Write(*currentTurn + 1);

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
	if (!playerTurnOrder.empty() && playerTurnOrder[*currentTurn].playerID != player->id)
		return *endRound;

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

	int option;
	std::cin >> option;

	//place organ
	if (option == 1)
	{
		player->hand.ListCards();
		std::cout << "Choose a card: ";
		int card;
		std::cin >> card;

		player->PlaceCard(card, Card::CardType::ORGAN, table, deck);

		std::cout << "Organ Placed!" << std::endl;
	}
	//infect other organ
	else if (option == 2)
	{

	}
	//vaccine organ
	else if (option == 3)
	{

	}
	//discard card
	else if (option == 4)
	{
		player->hand.ListCards();
		std::cout << "Choose a card: ";
		int card;
		std::cin >> card;
		player->hand.hand.erase(player->hand.hand.begin() + card);
		player->ReceiveCards(1, deck);
		std::cout << "Card Removed!" << std::endl;
	}
	//deploy threatment card
	else if (option == 5)
	{

	}

	std::cout << "waiting other players" << std::endl;

	UpdateTurn();

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
}

void GameManager::ReceiveMessages(TcpSocket* _sock, int* _sceneState)
{
	while (*_sceneState != (int)SceneManager::Scene::GAMEOVER) {
		Status status;
		InputMemoryStream* in = _sock->Receive(status);
		if (status == Status::DISCONNECTED)
		{
			delete in;
			return;
		}

		int instruction = 0;
		in->Read(&instruction);

		//Turn system
		if (instruction == (int)Commands::ORGAN_QUANTITY)
		{
			int playerID, organQuantity;

			in->Read(&playerID);

			in->Read(&organQuantity);

			if (playerTurnOrder.size() < socks->size() + 1)
				playerTurnOrder.push_back(Pair_Organ_Player(playerID, organQuantity));
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

			std::sort(playerTurnOrder.begin(), playerTurnOrder.end());
		}
		//Receive turn
		else if (instruction == (int)Commands::UPDATE_TURN)
		{
			int turn;
			in->Read(&turn);
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

		delete in;
	}
}

void GameManager::AcceptConnections(int* _sceneState)
{
	TcpListener listener;
	listener.Listen(localPort);

	while (socks->size() < 3 && *_sceneState != (int)SceneManager::Scene::GAMEOVER)
	{
		TcpSocket* sock = new TcpSocket();
		Status status = listener.Accept(*sock);
		if (status == Status::DONE) {
			socks->push_back(sock);
			table->table.push_back(std::vector<Card*>());
			std::cout << "Connected with ip: " << sock->GetRemoteAddress() << " and port: " << sock->GetLocalPort() << std::endl;

			/*TurnSystem(_socks);*/
			std::thread tReceive(&GameManager::ReceiveMessages, this, socks->back(), _sceneState);
			tReceive.detach();
		}
	}

	listener.Close();
}

void GameManager::CreateGame(TcpSocket* serverSock)
{
	Status status;
	InputMemoryStream* in = serverSock->Receive(status);
	if (status == Status::DONE)
	{
		std::string msg = in->ReadString();
		std::cout << msg << std::endl;

		OutputMemoryStream *out = new OutputMemoryStream();
		std::cin >> msg;
		out->WriteString(msg);

		serverSock->Send(out, status);
		delete out;
	}

	delete in;
}

void GameManager::ListCurrentGames(TcpSocket* serverSock)
{
	Status status;
	InputMemoryStream* inp = serverSock->Receive(status);
	int size;
	inp->Read(&size);
	for (int i = 0; i < size; i++)
	{
		int idx;
		inp->Read(&idx);
		int numOfPlayers;
		inp->Read(&numOfPlayers);
		std::cout << "Game number: " << idx << ", Players connected: " << numOfPlayers << std::endl;
	}

	delete inp;
}

void GameManager::JoinGame(TcpSocket* serverSock)
{
	//Choose game
	std::cout << "Type server ID" << std::endl;
	int server;
	std::cin >> server;

	OutputMemoryStream* out = new OutputMemoryStream();
	Status status;
	out->Write(server);
	serverSock->Send(out, status);

	delete out;

	//Write password (if necessary)
	InputMemoryStream* in;
	in = serverSock->Receive(status);

	std::string msg = in->ReadString();
	delete in;

	//Write message in console
	std::cout << msg << std::endl;

	if (msg != "")
	{
		do
		{
			in = serverSock->Receive(status);

			msg = in->ReadString();
			std::cout << msg << std::endl;

			std::cin >> msg;

			out = new OutputMemoryStream();
			out->WriteString(msg);
			serverSock->Send(out, status);
			delete out;

		} while (msg == "Incorrect password. Try again or write 'exit' to leave");
	}
}


void GameManager::ConnectP2P(TcpSocket* _serverSock, int* _sceneState)
{
	Status status;
	InputMemoryStream* in = _serverSock->Receive(status);

	int socketNum;
	in->Read(&socketNum);

	_serverSock->Disconnect();

	table->table.push_back(std::vector<Card*>());

	bool started = false;
	for (int i = 0; i < socketNum; i++)
	{
		PeerAddress address;
		TcpSocket* sock = new TcpSocket();
		std::string msg;

		int num;

		if (!started)
		{
			in->Read(&num);
			std::cout << num << std::endl;
			started = true;
		}

		msg = in->ReadString();
		in->Read(&address.port);
		std::cout << "Connected with ip: " << msg << " and port: " << address.port << std::endl;
		status = sock->Connect(msg, address.port);

		if (status == Status::DONE)
		{
			socks->push_back(sock);
			table->table.push_back(std::vector<Card*>());
			std::cout << "Connected with ip: " << sock->GetRemoteAddress() << " and port: " << sock->GetLocalPort() << std::endl;
		}
	}
	player->id = socks->size();
	delete in;

	std::thread tAccept(&GameManager::AcceptConnections, this, _sceneState);
	tAccept.detach();
	for (int i = 0; i < socks->size(); i++)
	{
		std::thread tReceive(&GameManager::ReceiveMessages, this, socks->at(i), _sceneState);
		tReceive.detach();
	}
}

