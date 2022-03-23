#include "GameManager.h"
#include "SceneManager.h"
#include <thread>
#include "../res/TcpSocket.h"

void GameManager::CalculateOrganQuantity()
{
	mtx.lock();
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

	mtx.unlock();

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
	//PROBLEMA AQUÍ!!!!!
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
		if (status == Status::DISCONNECTED)
		{
			return;
		}

		mtx.lock();

		int instruction = 0;
		in1.Read(&instruction);

		//Turn system
		if (instruction == (int)Commands::ORGAN_QUANTITY)
		{
			int playerID, organQuantity;

			in1.Read(&playerID);

			in1.Read(&organQuantity);

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

			std::sort(playerTurnOrder.begin(), playerTurnOrder.end(), ComparePlayers);
			//CheckArray();
		}
		//Receive turn
		else if (instruction == (int)Commands::UPDATE_TURN)
		{
			int turn;
			in1.Read(&turn);
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
			bool isReady;
			in1.Read(&isReady);
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
		mtx.unlock();
	}
}

void GameManager::AcceptConnections(int* _sceneState)
{
	TcpListener listener;
	listener.Listen(localPort);
	TcpSocket* sock;

	while (socks->size() <= 3 && *_sceneState != (int)SceneManager::Scene::GAMEOVER)
	{
		sock = new TcpSocket();
		Status status = listener.Accept(*sock);
		if (status == Status::DONE) {
			socks->push_back(sock);
			table->table.push_back(std::vector<Card*>());
			std::cout << "Connected with ip: " << sock->GetRemoteAddress() << " and port: " << sock->GetLocalPort() << std::endl;

			/*TurnSystem(_socks);*/
			std::thread tReceive(&GameManager::ReceiveMessages, this, sock, _sceneState);
			tReceive.detach();
		}
	}

	listener.Close();
}

void GameManager::CreateGame(TcpSocket* serverSock)
{
	std::cout << "c1" << std::endl;
	Status status;
	std::cout << "c2" << std::endl;
	InputMemoryStream* in = serverSock->Receive(status);
	std::cout << "c3" << std::endl;

	if (status == Status::DONE)
	{
		std::cout << "c4" << std::endl;
		if (in == nullptr)
			exit;
		std::cout << "c5" << std::endl;
		std::string msg = in->ReadString();
		std::cout << "c6" << std::endl;
		std::cout << msg << std::endl;
		std::cout << "c7" << std::endl;
		OutputMemoryStream *out = new OutputMemoryStream();
		std::cout << "c8" << std::endl;
		std::cin >> msg;
		std::cout << "c9" << std::endl;
		out->WriteString(msg);
		std::cout << "c10" << std::endl;
		serverSock->Send(out, status);
		std::cout << "c11" << std::endl;
		delete out;
		std::cout << "c12" << std::endl;
	}

	delete in;
	std::cout << "c13" << std::endl;
}

void GameManager::ListCurrentGames(TcpSocket* serverSock)
{
	Status status;
	InputMemoryStream* inp = serverSock->Receive(status);

	if (inp == nullptr)
		exit;

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

	char tmpOption;
	std::cin >> tmpOption;
	int server = tmpOption - '0';

	mtx.lock();

	OutputMemoryStream* out = new OutputMemoryStream();
	Status status;
	out->Write(server);
	serverSock->Send(out, status);

	delete out;

	//Write password (if necessary)
	InputMemoryStream* in;
	in = serverSock->Receive(status);

	if (in == nullptr)
		exit;

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

	mtx.unlock();
}


void GameManager::ConnectP2P(TcpSocket* _serverSock, int* _sceneState)
{
	std::cout << "1" << std::endl;
	Status status;
	std::cout << "2" << std::endl;
	std::cout << "3" << std::endl;
	InputMemoryStream* in = _serverSock->Receive(status);
	std::cout << "4" << std::endl;
	if (in == nullptr)
		exit;
	std::cout << "5" << std::endl;
	int socketNum;
	in->Read(&socketNum);
	mtx.lock();
	std::cout << "6" << std::endl;
	_serverSock->Disconnect();
	std::cout << "7" << std::endl;
	table->table.push_back(std::vector<Card*>());
	std::cout << "8" << std::endl;
	bool started = false;
	std::cout << "9" << std::endl;
	for (int i = 0; i < socketNum; i++)
	{
		std::cout << "10" << std::endl;
		PeerAddress address;
		std::cout << "11" << std::endl;
		TcpSocket* sock = new TcpSocket();
		std::cout << "12" << std::endl;
		std::string msg;
		std::cout << "13" << std::endl;
		int num;
		std::cout << "14" << std::endl;
		if (!started)
		{
			std::cout << "15" << std::endl;
			if (in == nullptr)
				exit;
			std::cout << "16" << std::endl;
			in->Read(&num);
			std::cout << "17" << std::endl;
			std::cout << num << std::endl;
			std::cout << "18" << std::endl;
			started = true;
			std::cout << "19" << std::endl;
		}
		std::cout << "20" << std::endl;
		if (in == nullptr)
			exit;
		std::cout << "21" << std::endl;
		msg = in->ReadString();
		std::cout << "22" << std::endl;
		in->Read(&address.port);
		std::cout << "23" << std::endl;
		std::cout << "Connected with ip: " << msg << " and port: " << address.port << std::endl;
		std::cout << "23" << std::endl;
		status = sock->Connect(msg, address.port);
		std::cout << "24" << std::endl;
		if (status == Status::DONE)
		{
			std::cout << "25" << std::endl;
			socks->push_back(sock);
			std::cout << "26" << std::endl;
			table->table.push_back(std::vector<Card*>());
			std::cout << "27" << std::endl;
			std::cout << "Connected with ip: " << sock->GetRemoteAddress() << " and port: " << sock->GetLocalPort() << std::endl;
			std::cout << "28" << std::endl;
		}
	}
	std::cout << "29" << std::endl;
	player->id = socks->size();
	std::cout << "30" << std::endl;
	delete in;
	std::cout << "31" << std::endl;
	mtx.unlock();
	std::cout << "32" << std::endl;
	std::thread tAccept(&GameManager::AcceptConnections, this, _sceneState);
	std::cout << "33" << std::endl;
	tAccept.detach();
	std::cout << "34" << std::endl;
	for (int i = 0; i < socks->size(); i++)
	{
		std::cout << "35 " << i << std::endl;
		std::thread tReceive(&GameManager::ReceiveMessages, this, socks->at(i), _sceneState);
		std::cout << "36 " << i << std::endl;
		tReceive.detach();
	}
}

