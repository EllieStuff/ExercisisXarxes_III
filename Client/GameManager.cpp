#include "GameManager.h"
#include "SceneManager.h"
#include <thread>
#include "../res/TcpSocket.h"

void GameManager::CalculateTurn()
{
}

GameManager::GameManager()
{
}

GameManager::~GameManager()
{
	for (int i = 0; i < socks->size(); i++)
		delete socks->at(i);
	delete socks;

	delete player;
	delete deck;
	delete table;
}

void GameManager::ConnectP2P(TcpSocket &_serverSock, int* _sceneState)
{
	Status status;
	InputMemoryStream* in = _serverSock.Receive(status);

	int socketNum;
	in->Read(&socketNum);

	_serverSock.Disconnect();

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

	std::thread tAccept(&AcceptConnections, _sceneState);
	tAccept.detach();
	std::thread tSend(&SendMessages, _sceneState);
	tSend.detach();
	for (int i = 0; i < socks->size(); i++)
	{
		std::thread tReceive(&ReceiveMessages, socks->at(i), _sceneState);
		tReceive.detach();
	}
}

void GameManager::SendMessages(int* _sceneState)
{
	player->ReceiveCards(3, deck);
	ListCards();

	while (*_sceneState != (int)SceneManager::Scene::GAMEOVER) {

		//Falla aqui
		TurnSystem(socks);

		std::cout << "Waiting For your turn" << std::endl;

		while (*turnNum != player->id)
		{
		}

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
			ListCards();
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
			ListCards();
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

		TurnRotation(socks);

		while (!(*endRound))
		{

		}

		*playersFinishedRound = 0;
		*endRound = false;
		*turnNum = 0;

		//Mostrar missatges de tota la ronda (events)

		//__________________________________

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
		if (instruction == 0)
		{
			for (size_t i = 0; i < socks->size(); i++)
			{
				if (i > 0)
				{
					in = _sock->Receive(status);
					in->Read(&instruction);
				}

				int playerNum = 0;

				in->Read(&playerNum);

				in->Read(&playerOrgans[playerNum - 1]);

				/*if (playerNum == 1)
					in->Read(&player1Organs);
				else if (playerNum == 2)
					in->Read(&player2Organs);
				else if (playerNum == 3)
					in->Read(&player3Organs);
				else if (playerNum == 4)
					in->Read(&player4Organs);*/
			}

			if (socks->size() > 0)
			{
				std::sort(playerOrgans.begin(), playerOrgans.end());

				//if (player1Organs > player2Organs && player1Organs > player3Organs && player1Organs > player4Organs)
				//	*turnNum = 1;
				//else if (player2Organs > player1Organs && player2Organs > player3Organs && player2Organs > player4Organs)
				//	*turnNum = 2;
				//else if (player3Organs > player2Organs && player3Organs > player1Organs && player3Organs > player4Organs)
				//	*turnNum = 3;
				//else if (player4Organs > player2Organs && player4Organs > player3Organs && player4Organs > player1Organs)
				//	*turnNum = 4;
				//else
				//	*turnNum = 1;
			}
		}
		//Receive turn
		else if (instruction == 1)
		{
			int turn;
			in->Read(&turn);
			*turnNum = turn;
			(*playersFinishedRound)++;
		}
		//Receive EndRound
		else if (instruction == 2)
		{
			bool _endRound;
			in->Read(&_endRound);
			*endRound = _endRound;
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
			std::thread tReceive(&ReceiveMessages, socks->back());
			tReceive.detach();
		}
	}

	listener.Close();
}

void GameManager::CreateGame(TcpSocket &serverSock)
{
	Status status;
	InputMemoryStream* in = serverSock.Receive(status);
	if (status == Status::DONE)
	{
		std::string msg = in->ReadString();
		std::cout << msg << std::endl;

		OutputMemoryStream *out = new OutputMemoryStream();
		std::cin >> msg;
		out->WriteString(msg);

		serverSock.Send(out, status);
		delete out;
	}

	delete in;
}

void GameManager::ListCurrentGames(TcpSocket& serverSock)
{
	Status status;
	InputMemoryStream* inp = serverSock.Receive(status);
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

void GameManager::JoinGame(TcpSocket &serverSock)
{
	//Choose game
	std::cout << "Type server ID" << std::endl;
	int server;
	std::cin >> server;

	OutputMemoryStream* out = new OutputMemoryStream();

	Status status;
	serverSock.Send(out, status);

	delete out;

	//Write password (if necessary)
	InputMemoryStream* in;
	in = serverSock.Receive(status);

	std::string msg = in->ReadString();
	delete in;

	//Write message in console
	std::cout << msg << std::endl;

	if (msg != "")
	{
		do
		{
			in = serverSock.Receive(status);

			msg = in->ReadString();
			std::cout << msg << std::endl;

			std::cin >> msg;

			out = new OutputMemoryStream();
			out->WriteString(msg);
			serverSock.Send(out, status);
			delete out;

		} while (msg == "Incorrect password. Try again or write 'exit' to leave");
	}
}


