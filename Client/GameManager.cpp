#include "GameManager.h"
#include "SceneManager.h"
#include <thread>
#include "../res/TcpSocket.h"

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

void GameManager::AcceptConnections(int* _sceneState)
{
	TcpListener listener;
	listener.Listen(localPort);
	TcpSocket* sock;

	while (socks->size() <= 3 && *_sceneState != (int)SceneManager::Scene::GAMEOVER)
	{
		sock = new TcpSocket();
		Status status = listener.Accept(*sock);
		mtx.lock();
		if (status == Status::DONE) {
			std::cout << "Connected with ip: " << sock->GetRemoteAddress() << " and port: " << sock->GetLocalPort() << std::endl;
			socks->push_back(sock);
			table->table.push_back(std::vector<Card*>());

			/*TurnSystem(_socks);*/
			std::thread tReceive(&GameManager::ReceiveMessages, this, sock, _sceneState);
			tReceive.detach();
		}
		mtx.unlock();
	}

	listener.Close();
}

void GameManager::CreateGame(TcpSocket* _serverSock)
{
	std::string gameName, gamePassword;
	int numOfPlayers;
	std::cout << "Game's name: " << std::endl;
	std::cin >> gameName;
	do {
		char numOfPlayersChar;
		std::cout << "Game's number of players (2-4): " << std::endl;
		std::cin >> numOfPlayersChar;
		numOfPlayers = numOfPlayersChar - '0';
	} while (numOfPlayers < 2 || numOfPlayers > 4);
	//mtx.lock();
	bool passwordAssigned = false;
	while(!passwordAssigned) {
		std::string ans;
		std::cout << "Do you want a password? (Y/N)" << std::endl;
		std::cin >> ans;
		if (ans == "Y" || ans == "y") {
			passwordAssigned = true;
			std::cout << "Write Your Password" << std::endl;
			std::cin >> gamePassword;
		}
		else if (ans == "N" || ans == "n") {
			passwordAssigned = true;
			gamePassword = "";
		}
	}

	//mtx.unlock();
	Status status;
	OutputMemoryStream out;
	out.Write((int)Commands::CREATE_GAME);
	out.WriteString(gameName);
	out.Write(numOfPlayers);
	out.WriteString(gamePassword);
	_serverSock->Send(&out, status);
}

void GameManager::ListCurrentGames(TcpSocket* _serverSock)
{
	Status status;
	InputMemoryStream* inp = _serverSock->Receive(status);

	if (status == Status::DONE)
	{
		mtx.lock();
		int size;
		inp->Read(&size);
		mtx.unlock();
		for (int i = 0; i < size; i++)
		{
			mtx.lock();
			int idx;
			inp->Read(&idx);
			int numOfPlayers;
			inp->Read(&numOfPlayers);
			mtx.unlock();
			std::cout << "Game number: " << idx << ", Players connected: " << numOfPlayers << std::endl;
		}

		delete inp;
	}
}

void GameManager::JoinGame(TcpSocket* _serverSock, bool& _aborted)
{
	Status status;
	OutputMemoryStream* out;
	InputMemoryStream* in;
	bool validIdx = false;
	do {
		//Choose game
		std::cout << "Type server ID" << std::endl;
		//char tmpOption;
		int serverIdx;// = tmpOption - '0';
		std::cin >> serverIdx;

		mtx.lock();
		out = new OutputMemoryStream();
		out->Write(serverIdx);
		_serverSock->Send(out, status);
		delete out;
		if (serverIdx == -1) {
			_aborted = true;
			mtx.unlock();
			return;
		}
		mtx.unlock();

		in = _serverSock->Receive(status);
		mtx.lock();
		in->Read(&validIdx);
		delete in;
		mtx.unlock();
	} while (!validIdx);


	//Write password (if necessary)
	in = _serverSock->Receive(status);

	if (status == Status::DONE) 
	{
		std::string msg = in->ReadString();
		delete in;
		//Write message in console
		std::cout << msg << std::endl;

		if (msg != "")
		{
			bool validPassword = false;
			std::string password = "";
			do
			{
				//if (validPassword)
				//	exit;

				std::cin >> password;
				OutputMemoryStream* out2 = new OutputMemoryStream();
				out2->WriteString(password);
				_serverSock->Send(out2, status);
				delete out2;
				if (password == "exit") {
					_aborted = true;
					return;
				}

				in = _serverSock->Receive(status);

				mtx.lock();
				if (status != Status::DONE) {
					_aborted = true;
					return;
				}

				in->Read(&validPassword);
				delete in;
				mtx.unlock();

				if(!validPassword)
					std::cout << "Write the password. Write exit to leave\n";

			} while (!validPassword);
		}
	}
}


void GameManager::ConnectP2P(TcpSocket* _serverSock, int* _sceneState)
{
	Status status;
	InputMemoryStream in = *_serverSock->Receive(status);

	if(status == Status::DONE) 
	{
		mtx.lock();
		int socketNum;
		in.Read(&socketNum);
		_serverSock->Disconnect();
		table->table.push_back(std::vector<Card*>());
		mtx.unlock();
		bool started = false;
		for (int i = 0; i < socketNum; i++)
		{
			PeerAddress address;
			TcpSocket* sock = new TcpSocket();
			std::string msg;
			int num;
			if (!started)
			{
				mtx.lock();
				in.Read(&num);
				mtx.unlock();
				started = true;
			}
			mtx.lock();
			msg = in.ReadString();
			in.Read(&address.port);
			mtx.unlock();
			std::cout << "Connected with ip: " << msg << " and port: " << address.port << std::endl;
			status = sock->Connect(msg, address.port);
			if (status == Status::DONE)
			{
				mtx.lock();
				socks->push_back(sock);
				table->table.push_back(std::vector<Card*>());
				mtx.unlock();
				std::cout << "Connected with ip: " << sock->GetRemoteAddress() << " and port: " << sock->GetLocalPort() << std::endl;
			}
		}
		player->id = socks->size();
		std::thread tAccept(&GameManager::AcceptConnections, this, _sceneState);
		tAccept.detach();
		for (int i = 0; i < socks->size(); i++)
		{
			std::thread tReceive(&GameManager::ReceiveMessages, this, socks->at(i), _sceneState);
			tReceive.detach();
		}
	}
}

