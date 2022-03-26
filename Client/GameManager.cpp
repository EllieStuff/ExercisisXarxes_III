#include "GameManager.h"
#include "SceneManager.h"
#include <thread>

void GameManager::CalculateOrganQuantity()
{
	int organQuantity = 0;
	for (size_t i = 0; i < player->hand.hand.size(); i++)
	{
		mtx.lock();
		if (player->hand.hand[i]->cardType == Card::CardType::ORGAN)
			organQuantity++;
		mtx.unlock();
	}

	if (playerTurnOrder.size() < socks->size() + 1) 
	{
		mtx.lock();
		playerTurnOrder.push_back(Pair_Organ_Player(player->id, organQuantity));
		mtx.unlock();
	}
	else
	{
		mtx.lock();
		for (size_t i = 0; i < playerTurnOrder.size(); i++)
		{
			if (playerTurnOrder[i].playerID == player->id) 
			{
				playerTurnOrder[i].numOrgans = organQuantity;
				break;
			}
		}
		mtx.unlock();
	}

	mtx.lock();
	std::sort(playerTurnOrder.begin(), playerTurnOrder.end(), ComparePlayers);
	mtx.unlock();

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

	for (std::list<TcpSocket*>::iterator it = socks->begin(); it != socks->end(); ++it)
	{
		Status status;
		TcpSocket& client = **it;
		client.Send(out, status);
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

	for (std::list<TcpSocket*>::iterator it = socks->begin(); it != socks->end(); ++it)
	{
		Status status;
		TcpSocket& client = **it;
		client.Send(out, status);
	}

	delete out;
}

GameManager::~GameManager()
{
	delete socks;

	delete player;
	delete deck;
	delete table;

	delete endRound;
	delete currentTurn;
}

void GameManager::ListEnemiesWithTheirCards() 
{
	OutputMemoryStream* out = new OutputMemoryStream();

	out->Write((int)Commands::LISTCARDS);
	out->Write(player->id);

	Status status;

	for (std::list<TcpSocket*>::iterator it = socks->begin(); it != socks->end(); ++it)
	{
		Status status;
		TcpSocket& client = **it;
		client.Send(out, status);
		//break;
	}

	delete out;
}

bool GameManager::PlaceInfection()
{
	int objective;
	std::cout << "Choose an objective ('3' to go back)" << std::endl;
	ListEnemiesWithTheirCards();
	std::cin >> objective;
	if (objective >= 3)
		return false;
	int card;
	player->hand.ListCards();
	std::cout << "Choose your card ('3' to go back)" << std::endl;
	std::cin >> card;
	if (card >= 3)
		return false;

	int organType = (int) player->hand.hand[card]->organType;

	OutputMemoryStream* out = new OutputMemoryStream();

	out->Write((int)Commands::PLACE_INFECTION);
	out->Write(objective);
	out->Write(organType);

	Status status;

	for (std::list<TcpSocket*>::iterator it = socks->begin(); it != socks->end(); ++it)
	{
		Status status;
		TcpSocket& client = **it;
		client.Send(out, status);
	}

	player->hand.hand.erase(player->hand.hand.begin() + card);

	return true;

	delete out;
}

bool GameManager::Update()
{
	if (*currentTurn == playerTurnOrder.size() || playerTurnOrder[*currentTurn].playerID != player->id)
		return *endRound;

	for (size_t i = 0; i < playerTurnOrder.size(); i++)
		std::cout << "turn player: " << playerTurnOrder[i].playerID << std::endl;

	bool finishedRound = false;

	while (!finishedRound)
	{
		player->ReceiveCards(MAX_CARDS - player->hand.hand.size(), deck);

		system("CLS");

		std::cout << "" << std::endl;
		std::cout << "___________HAND___________" << std::endl;
		player->hand.ListCards();
		std::cout << "___________HAND___________" << std::endl;

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
			std::cout << "Choose a card: ('3' to go back)";

			std::cin >> card;

			if(card >= 3)
				finishedRound = false;

			if(card < 3 && player->PlaceCard(card, Card::CardType::ORGAN, table, deck) == true) 
			{
				finishedRound = true;
				std::cout << "Organ Placed!" << std::endl;
				break;
			}

			break;
		case Commands::PLACE_INFECTION:
			if(PlaceInfection() == true) 
			{
				finishedRound = true;
				break;
			}
			finishedRound = false;
			break;
		case Commands::PLACE_MEDICINE:
			break;
		case Commands::PLACE_TREATMENT:
			break;
		case Commands::DISCARD_CARD:
			player->hand.ListCards();
			std::cout << "Choose a card: ('3' to go back)";

			std::cin >> card;

			if (card >= 3)
				break;

			player->hand.hand.erase(player->hand.hand.begin() + card);
			std::cout << "Card Removed!" << std::endl;
			finishedRound = true;
			break;
		default:
			break;
		}
	}

	std::cout << "waiting other players" << std::endl;

	UpdateTurn(true);

	//Mostrar missatges de tota la ronda (events)

	//__________________________________

	std::cout << "Waiting For your turn" << std::endl;

	return *endRound;
}

void GameManager::Start()
{
	player->ReceiveCards(3, deck);
	CalculateOrganQuantity();
	UpdateTurn(false);
}

void GameManager::SendReady()
{
	OutputMemoryStream* out = new OutputMemoryStream();
	out->Write(5);
	out->Write(true);

	Status status;

	for (std::list<TcpSocket*>::iterator it = socks->begin(); it != socks->end(); ++it)
	{
		Status status;
		TcpSocket& client = **it;
		client.Send(out, status);
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

void GameManager::ReceiveMessages(InputMemoryStream in1)
{
	//while (true) {

		//mtx.lock();
		int instruction = 0;
		in1.Read(&instruction);
		//mtx.unlock();

		//Turn system
		if (instruction == (int)Commands::ORGAN_QUANTITY)
		{
			//mtx.lock();
			int playerID, organQuantity;

			in1.Read(&playerID);

			in1.Read(&organQuantity);
			//mtx.unlock();

			if (playerTurnOrder.size() < socks->size() + 1) 
			{
				mtx.lock();
				playerTurnOrder.push_back(Pair_Organ_Player(playerID, organQuantity));
				mtx.unlock();
			}
			else
			{
				mtx.lock();
				for (size_t i = 0; i < playerTurnOrder.size(); i++)
				{
					if (playerTurnOrder[i].playerID == playerID)
					{
						playerTurnOrder[i].numOrgans = organQuantity;
						break;
					}
				}
				mtx.unlock();
			}

			mtx.lock();
			std::sort(playerTurnOrder.begin(), playerTurnOrder.end(), ComparePlayers);
			mtx.unlock();
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
				mtx.lock();
				*endRound = true;
				mtx.unlock();
			}
				
			*currentTurn = turn;
		}
		else if (instruction == (int)Commands::PLACE_INFECTION)
		{
			int playerID;
			in1.Read(&playerID);

			if(playerID == player->id) 
			{
				int organType;
				in1.Read(&organType);

				for (size_t i = 0; i < table->table.size(); i++)
				{
					for (size_t o = 0; o < table->table[i].size(); o++)
					{
						if (table->table.at(i).at(o)->organType == (Card::OrganType)organType || organType == (int) Card::OrganType::NONE)
						{
							table->table.at(i).at(o)->virusQuantity += 1;
							system("CLS");
							std::cout << "Virus Received!!" << std::endl;
							break;
						}
					}
				}
			}
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
		else if(instruction == (int)Commands::LISTCARDS) 
		{
			int playerID;
			in1.Read(&playerID);

			OutputMemoryStream out;

			out.Write((int)Commands::SHOWCARDS);

			out.Write(player->id);

			mtx.lock();

			for (size_t i = 0; i < table->table.size(); i++)
			{
				if (table->table[i].size() > 0)
				{
					int size = table->table.at(i).size();
					std::cout << size << std::endl;
					out.Write(size);
					break;
				}
			}

			for (size_t i = 0; i < table->table.size(); i++)
			{
				if (table->table[i].size() > 0)
				{
					for (size_t o = 0; o < table->table.at(i).size(); o++)
					{
						out.Write((int)table->table[i][o]->cardType);
						out.Write((int)table->table[i][o]->organType);
						out.Write((int)table->table[i][o]->virusQuantity);
					}
				}
			}

			mtx.unlock();

			int index = 0;

			for (std::list<TcpSocket*>::iterator it = socks->begin(); it != socks->end(); ++it)
			{
				Status status;
				TcpSocket& client = **it;
				client.Send(&out, status);
				index++;
				//break;
			}


		}
		else if(instruction == (int)Commands::SHOWCARDS) 
		{
			int playerID;
			in1.Read(&playerID);
			int tableSize;
			in1.Read(&tableSize);
			for (size_t i = 0; i < tableSize; i++)
			{
				int cardType;
				in1.Read(&cardType);

				int organType;
				in1.Read(&organType);

				int virusQuantity;
				in1.Read(&virusQuantity);

				std::string _cardType;
				std::string _organType;
				std::string _threatmentType;

				switch (cardType)
				{
				case 0:
					_cardType = "ORGAN";
					break;
				case 1:
					_cardType = "MEDICINE";
					break;
				case 2:
					_cardType = "VIRUS";
					break;
				case 3:
					_cardType = "TREATMENT";
					break;
				}

				switch (organType)
				{
				case 0:

					_organType = "STOMACH";
					break;
				case 1:
					_organType = "BRAIN";
					break;
				case 2:
					_organType = "SKELETON";
					break;
				case 3:
					_organType = "HEART";
					break;
				case 4:
					_organType = "NONE";
					break;
				}

				std::cout << "PLAYER: " << playerID << " | CARD: " << _cardType << " ORGAN: " << _organType << " VIRUS: " << virusQuantity << std::endl;
			}
		}
}

void GameManager::AcceptConnections(int* _sceneState)
{
	listener.Listen(localPort);
	TcpSocket* sock;

	while (*_sceneState != (int)SceneManager::Scene::GAMEOVER)
	{
		if(listener.selector.wait()) 
		{
			if (listener.selector.isReady(listener.listener))
			{
				sock = new TcpSocket();
				Status status = listener.Accept(*sock);
				if (status == Status::DONE)
				{
					socks->push_back(sock);
					listener.selector.add(sock->socket);
					table->table.push_back(std::vector<Card*>());
					std::cout << "Connected with ip: " << sock->GetRemoteAddress() << " and port: " << sock->GetLocalPort() << std::endl;
				}
				else
				{
					delete sock;
				}
			}
			else 
			{
				for (std::list<TcpSocket*>::iterator it = socks->begin(); it != socks->end(); ++it) 
				{
					TcpSocket& client = **it;
					if (listener.selector.isReady(client.socket))
					{
						Status status;
						InputMemoryStream in = *client.Receive(status);
						if (status == Status::DONE)
							GameManager::ReceiveMessages(in);
						if (status == Status::DISCONNECTED)
							listener.selector.remove(client.socket);
					}
				}
			}
		}
	}

	listener.Close();
}

void GameManager::CreateGame(TcpSocket* serverSock)
{
	mtx.lock();
	std::string msg = "Write Your Password (type '-' to leave it empty)";
	std::cout << msg << std::endl;
	mtx.unlock();
	Status status;
	std::string msg2;
	std::cin >> msg2;
	OutputMemoryStream out;
	out.WriteString(msg2);
	serverSock->Send(&out, status);
}

void GameManager::ListCurrentGames(TcpSocket* serverSock)
{
	Status status;
	InputMemoryStream* inp = serverSock->Receive(status);

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

void GameManager::JoinGame(TcpSocket* serverSock)
{
	Status status;
	OutputMemoryStream* out;
	InputMemoryStream* in;
	bool validIdx = false;
	do {
		//Choose game
		std::cout << "Type server ID" << std::endl;
		char tmpOption;
		std::cin >> tmpOption;
		int serverIdx = tmpOption - '0';

		mtx.lock();
		out = new OutputMemoryStream();
		out->Write(serverIdx);
		serverSock->Send(out, status);
		delete out;
		mtx.unlock();

		in = serverSock->Receive(status);
		mtx.lock();
		in->Read(&validIdx);
		delete in;
		mtx.unlock();
	} while (!validIdx);


	//Write password (if necessary)
	in = serverSock->Receive(status);

	if (status == Status::DONE) 
	{
		std::string msg = in->ReadString();
		delete in;
		//Write message in console
		std::cout << msg << std::endl;

		if (msg != "")
		{
			bool validPassword = false;
			std::string msg3 = "";
			do
			{
				if (validPassword)
					exit;

				std::cin >> msg3;
				OutputMemoryStream* out2 = new OutputMemoryStream();
				out2->WriteString(msg3);
				serverSock->Send(out2, status);
				delete out2;

				in = serverSock->Receive(status);

				mtx.lock();
				if (status != Status::DONE)
					break;

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
				listener.selector.add(sock->socket);
				table->table.push_back(std::vector<Card*>());
				mtx.unlock();
				std::cout << "Connected with ip: " << sock->GetRemoteAddress() << " and port: " << sock->GetLocalPort() << std::endl;
			}
		}
		player->id = socks->size();
		std::thread tAccept(&GameManager::AcceptConnections, this, _sceneState);
		tAccept.detach();
		while(true) 
		{
			if (listener.selector.wait())
			{
				if (!listener.selector.isReady(listener.listener)) 
				{
					for (std::list<TcpSocket*>::iterator it = socks->begin(); it != socks->end(); ++it)
					{
						TcpSocket& client = **it;
						if (listener.selector.isReady(client.socket))
						{
							Status status;
							InputMemoryStream in = *client.Receive(status);
							if (status == Status::DONE)
								GameManager::ReceiveMessages(in);
							if (status == Status::DISCONNECTED)
								listener.selector.remove(client.socket);
						}
					}
				}
			}
		}
	}
}

