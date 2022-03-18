
#include <iostream>
#include <vector>
#include <thread>
#include "..\res\TcpSocket.h"
#include "..\res\TcpListener.h"
#include "..\res\Utils.h"
#include <mutex>

std::mutex mtx;

//Cards System
std::vector<Card*> organs;
std::vector<Card*> medicines;
std::vector<Card*> virus;
std::vector<Card*> threatment;
std::vector<Card*> hand;
std::vector<Card*> table;
std::vector<Card*> deck;
std::vector<Card*> deckScrambled;

std::vector<std::vector<Card*>> cardsOnEveryOrgan;
//___________________________

//Game System
int playerNum;
int* turnNum = new int(0);

int player1Organs = 0;
int player2Organs = 0;
int player3Organs = 0;
int player4Organs = 0;

bool* endRound = new bool(false);
int* playersFinishedRound = new int(0);
//___________________________

// Mirar ip en consola amb ipconfig, sino, es pot fer en mateix PC amb "127.0.0.1" o "localHost"

unsigned short localPort;
bool endGame = false;

void ShowTable() 
{
	for (size_t i = 0; i < table.size(); i++)
	{
		std::string cardType;
		std::string organType;
		std::string threatmentType;

		switch ((int)table.at(i)->cardType)
		{
		case 0:
			cardType = "ORGAN";
			break;
		case 1:
			cardType = "MEDICINE";
			break;
		case 2:
			cardType = "VIRUS";
			break;
		case 3:
			cardType = "TREATMENT";
			break;
		}

		switch ((int)table.at(i)->organType)
		{
		case 0:
			
			organType = "STOMACH";
			break;
		case 1:
			organType = "BRAIN";
			break;
		case 2:
			organType = "SKELETON";
			break;
		case 3:
			organType = "HEART";
			break;
		case 4:
			organType = "NONE";
			break;
		}

		switch ((int)table.at(i)->treatmentType)
		{
		case 0:
			threatmentType = "INFECTION";
			break;
		case 1:
			threatmentType = "ROBER";
			break;
		case 2:
			threatmentType = "TRANSPLANT";
			break;
		case 3:
			threatmentType = "LATEX_GLOVES";
			break;
		case 4:
			threatmentType = "MEDICAL_ERROR";
			break;
		case 5:
			threatmentType = "NONE";
			break;
		}

		std::cout << "Card: " + std::to_string(i) + " cardType: " + cardType + " OrganType: " + organType + " ThreatmentType: " + threatmentType << std::endl;
	}
}

void listCards()
{
	for (size_t i = 0; i < hand.size(); i++)
	{
		std::string cardType;
		std::string organType;
		std::string threatmentType;

		switch ((int)hand.at(i)->cardType)
		{
		case 0:
			cardType = "ORGAN";
			break;
		case 1:
			cardType = "MEDICINE";
			break;
		case 2:
			cardType = "VIRUS";
			break;
		case 3:
			cardType = "TREATMENT";
			break;
		}

		switch ((int)hand.at(i)->organType)
		{
		case 0:
			organType = "STOMACH";
			break;
		case 1:
			organType = "BRAIN";
			break;
		case 2:
			organType = "SKELETON";
			break;
		case 3:
			organType = "HEART";
			break;
		case 4:
			organType = "NONE";
			break;
		}

		switch ((int)hand.at(i)->treatmentType)
		{
		case 0:
			threatmentType = "INFECTION";
			break;
		case 1:
			threatmentType = "ROBER";
			break;
		case 2:
			threatmentType = "TRANSPLANT";
			break;
		case 3:
			threatmentType = "LATEX_GLOVES";
			break;
		case 4:
			threatmentType = "MEDICAL_ERROR";
			break;
		case 5:
			threatmentType = "NONE";
			break;
		}
		if (cardType._Equal("TREATMENT"))
			std::cout << "Card: " << i << " cardType: " << cardType << " ThreatmentType: " << threatmentType << std::endl;
		else
			std::cout << "Card: " << i << " cardType: " << cardType << " OrganType: " << organType << std::endl;
	}
}


void receiveCards(int quantity)
{
	for (size_t i = 0; i < quantity; i++)
	{
		hand.push_back(deckScrambled.at(deckScrambled.size() - 1));
		deckScrambled.erase(deckScrambled.end() - 1);
	}
	listCards();
}


void PlaceCard(int pos, Card::CardType type) 
{
	if(hand[pos]->cardType == type) 
	{
		table.push_back(hand[pos]);
		hand.erase(hand.begin() + pos);
		receiveCards(1);
	}
	else 
	{
		std::cout << "Invalid Card!!!!!" << std::endl;
	}
}

void TurnSystem(std::vector<TcpSocket*>* _socks)
{
	mtx.lock();
	int organQuantity = 0;
	for (size_t i = 0; i < hand.size(); i++)
	{
		if (hand.at(i)->cardType == Card::CardType::ORGAN)
			organQuantity++;
	}

	if (playerNum == 1)
		player1Organs = organQuantity;
	else if (playerNum == 2)
		player2Organs = organQuantity;
	else if (playerNum == 3)
		player3Organs = organQuantity;
	else if (playerNum == 4)
		player4Organs = organQuantity;

	OutputMemoryStream* out = new OutputMemoryStream();
	//instruction 0: receive the organ quantity to receive the turn
	out->Write(0);
	out->Write(playerNum);
	out->Write(organQuantity);

	std::cout << organQuantity << std::endl;

	for (int i = 0; i < _socks->size(); i++)
	{
		Status status;
		_socks->at(i)->Send(out, status);
	}

	delete(out);

	mtx.unlock();
}

void TurnRotation(std::vector<TcpSocket*>* _socks)
{
	OutputMemoryStream* out = new OutputMemoryStream();

	(*playersFinishedRound)++;

	for (size_t i = 0; i < _socks->size() + 1; i++)
	{
		if(i != playerNum - 1) 
		{
			//instruction 1: send your turn to another player
			out->Write(1);
			out->Write(i+1);
			break;
		}
	}

	Status status;

	for (int i = 0; i < _socks->size(); i++)
	{
		_socks->at(i)->Send(out, status);
	}

	if (*playersFinishedRound > _socks->size())
	{
		for (int i = 0; i < _socks->size(); i++)
		{
			delete out;
			out = new OutputMemoryStream();
			out->Write(2);
			out->Write(true);
			_socks->at(i)->Send(out, status);
		}
	}

	*endRound = true;

	delete(out);
}

void SendMessages(std::vector<TcpSocket*>* _socks) {
	playerNum = _socks->size()+1;
	receiveCards(3);
	
	while (!endGame) {

		//Falla aqui
		TurnSystem(_socks);

		std::cout << "Waiting For your turn" << std::endl;

		while (*turnNum != playerNum)
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
		ShowTable();
		std::cout << "___________TABLE___________" << std::endl;
		std::cout << "" << std::endl;

		int option;
		std::cin >> option;

		//place organ
		if(option == 1)
		{
			listCards();
			std::cout << "Choose a card: ";
			int card;
			std::cin >> card;

			PlaceCard(card, Card::CardType::ORGAN);

			std::cout << "Organ Placed!" << std::endl;
		}
		//infect other organ
		else if(option == 2) 
		{

		}
		//vaccine organ
		else if (option == 3) 
		{

		}
		//discard card
		else if (option == 4)
		{
			listCards();
			std::cout << "Choose a card: ";
			int card;
			std::cin >> card;
			hand.erase(hand.begin() + card);
			receiveCards(1);
			std::cout << "Card Removed!" << std::endl;
		}
		//deploy threatment card
		else if (option == 5)
		{

		}

		std::cout << "waiting other players" << std::endl;

		TurnRotation(_socks);

		while(!(*endRound)) 
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

void ReceiveMessages(std::vector<TcpSocket*>* _socks, TcpSocket* _sock) {
	while (!endGame) {
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
		if(instruction == 0) 
		{
			for (size_t i = 0; i < _socks->size(); i++)
			{
				if(i > 0) 
				{
					in = _sock->Receive(status);
					in->Read(&instruction);
				}

				int playerNum = 0;

				in->Read(&playerNum);

				if (playerNum == 1)
					in->Read(&player1Organs);
				else if (playerNum == 2)
					in->Read(&player2Organs);
				else if (playerNum == 3)
					in->Read(&player3Organs);
				else if (playerNum == 4)
					in->Read(&player4Organs);
			}

			if (_socks->size() > 0)
			{
				if (player1Organs > player2Organs && player1Organs > player3Organs && player1Organs > player4Organs)
					*turnNum = 1;
				else if (player2Organs > player1Organs && player2Organs > player3Organs && player2Organs > player4Organs)
					*turnNum = 2;
				else if (player3Organs > player2Organs && player3Organs > player1Organs && player3Organs > player4Organs)
					*turnNum = 3;
				else if (player4Organs > player2Organs && player4Organs > player3Organs && player4Organs > player1Organs)
					*turnNum = 4;
				else
					*turnNum = 1;
			}
		}
		//Receive turn
		else if(instruction == 1) 
		{
			int turn;
			in->Read(&turn);
			*turnNum = turn;
			(*playersFinishedRound)++;
		}
		//Receive EndRound
		else if(instruction == 2) 
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

void AcceptPeers(std::vector<TcpSocket*>* _socks) {
	TcpListener listener;
	listener.Listen(localPort);

	while (_socks->size() < 3 && !endGame)
	{
		TcpSocket* sock = new TcpSocket();
		Status status = listener.Accept(*sock);
		if (status == Status::DONE) {
			_socks->push_back(sock);
			std::cout << "Connected with ip: " << sock->GetRemoteAddress() << " and port: " << sock->GetLocalPort() << std::endl;

			TurnSystem(_socks);
			std::thread tReceive(ReceiveMessages, _socks, _socks->at(_socks->size() - 1));
			tReceive.detach();
		}
	}

	listener.Close();

}

void ConnectPeer2Peer(std::vector<TcpSocket*>* _socks) 
{
	TcpSocket serverSock;
	Status status = serverSock.Connect("127.0.0.1", 50000);
	if (status != Status::DONE) {
		return;
	}
	localPort = serverSock.GetLocalPort();

	std::cout << "Welcome!" << std::endl;
	std::cout << "1. Create P2P Game" << std::endl;
	std::cout << "2. List P2P Games" << std::endl;
	std::cout << "3. Join P2P Game" << std::endl;

	while (true)
	{
		std::cout << "\nSelect option: ";
		int option;
		std::cin >> option;
		if (option <= 0 || option > 3)
			continue;

		OutputMemoryStream* out0 = new OutputMemoryStream();
		out0->Write(option);

		serverSock.Send(out0, status);
		delete out0;
		out0 = new OutputMemoryStream();

		if (option == 3)
		{
			std::cout << "Type server number" << std::endl;
			int server;
			std::cin >> server;

			out0->Write(server);
			serverSock.Send(out0, status);
			delete out0;
			out0 = new OutputMemoryStream();

			InputMemoryStream* in0;
			
			//rep el missatge de server protected with password
			in0 = serverSock.Receive(status);

			std::string msg = in0->ReadString();

			std::cout << msg << std::endl;

			if (msg != "")
			{
				do
				{
					in0 = serverSock.Receive(status);

					msg = in0->ReadString();
					std::cout << msg << std::endl;

					std::cin >> msg;

					out0->WriteString(msg);
					serverSock.Send(out0, status);

				} while (msg == "Incorrect password. Try again or write 'exit' to leave");

			}

			delete in0;
			delete out0;
		}
		else if (option == 2)
		{
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
		else if (option == 1)
		{
			InputMemoryStream* in = serverSock.Receive(status);
			if (status == Status::DONE)
			{
				std::string msg = in->ReadString();
				std::cout << msg << std::endl;

				std::cin >> msg;
				out0->WriteString(msg);

				serverSock.Send(out0, status);
			}

			delete in;
		}

		if (option == 1 || option == 3)
		{
			InputMemoryStream* in = serverSock.Receive(status);

			int socketNum;
			in->Read(&socketNum);

			serverSock.Disconnect();

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
					_socks->push_back(sock);
					std::cout << "Connected with ip: " << sock->GetRemoteAddress() << " and port: " << sock->GetLocalPort() << std::endl;
				}
			}
			delete in;

			std::thread tAccept(AcceptPeers, _socks);
			tAccept.detach();
			std::thread tSend(SendMessages, _socks);
			tSend.detach();
			for (int i = 0; i < _socks->size(); i++)
			{
				std::thread tReceive(ReceiveMessages, _socks, _socks->at(i));
				tReceive.detach();
			}
			break;
		}
	}
}

void InitializeCards()
{
	Card* organCard;
	Card* medicineCard;
	Card* virusCard;
	Card* threatmentCard;

	for (size_t i = 0; i < 5; i++)
	{
		organCard = new Card();
		organCard->cardType = (Card::CardType)(int)0;
		organCard->organType = (Card::OrganType)(int)i;
		organs.push_back(organCard);

		medicineCard = new Card();
		medicineCard->cardType = (Card::CardType)(int)1;
		medicineCard->organType = (Card::OrganType)(int)i;
		medicines.push_back(medicineCard);

		virusCard = new Card();
		virusCard->cardType = (Card::CardType)(int)2;
		virusCard->organType = (Card::OrganType)(int)i;
		virus.push_back(medicineCard);
	}

	for (size_t i = 0; i < 6; i++)
	{
		Card* threatmentCard = new Card();
		threatmentCard->cardType = (Card::CardType)(int)3;
		threatmentCard->treatmentType = (Card::TreatmentType)(int)i;
		threatment.push_back(threatmentCard);
	}

	//Deck Creation
	deck.push_back(organs.at(4));

	for (size_t i = 0; i < organs.size() - 1; i++)
	{
		for (size_t o = 0; o < 5; o++)
			deck.push_back(organs.at(i));
	}

	deck.push_back(virus.at(4));

	for (size_t i = 0; i < virus.size() - 1; i++)
	{
		for (size_t o = 0; o < 4; o++)
			deck.push_back(organs.at(i));
	}

	for (size_t i = 0; i < medicines.size(); i++)
	{
		for (size_t o = 0; o < 4; o++)
			deck.push_back(medicines.at(i));
	}

	for (size_t i = 0; i < 2; i++)
		deck.push_back(threatment.at(0));


	for (size_t i = 0; i < 3; i++)
	{
		deck.push_back(threatment.at(1));
		deck.push_back(threatment.at(2));
	}

	deck.push_back(threatment.at(3));
	deck.push_back(threatment.at(4));

	deckScrambled = deck;

	srand((unsigned int)time(NULL));
	std::random_shuffle(std::begin(deckScrambled), std::end(deckScrambled));

	//End of Deck Creation
}


int main() {

	std::vector<TcpSocket*> socks;
	InitializeCards();
	ConnectPeer2Peer(&socks);

	while(!endGame){}

	return 0;
}