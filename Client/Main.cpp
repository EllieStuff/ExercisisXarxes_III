
#include <iostream>
#include <vector>
#include <thread>
#include "..\res\TcpSocket.h"
#include "..\res\TcpListener.h"
#include "..\res\Utils.h"

//Cards System
std::vector<Card*> organs;
std::vector<Card*> medicines;
std::vector<Card*> virus;
std::vector<Card*> threatment;
std::vector<Card*> hand;
std::vector<Card*> table;
std::vector<std::vector<Card*>> cardsOnEveryOrgan;
//___________________________

//Game System
int playerNum;
int turNum;
//___________________________

// Mirar ip en consola amb ipconfig, sino, es pot fer en mateix PC amb "127.0.0.1" o "localHost"

unsigned short localPort;
bool end = false;

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

		std::cout << "Card: " + std::to_string(i) + " cardType: " + cardType + " OrganType: " + organType + " ThreatmentType: " + threatmentType << std::endl;
	}
}

void receiveCards(int quantity) 
{
	srand((unsigned int)time(NULL));
	for (size_t i = 0; i < quantity; i++)
	{
		int random = (rand() % (4 + 0));
		int random2;

		switch(random) 
		{
			case 0:
				random2 = (rand() % (organs.size() + 0));
				hand.push_back(organs.at(random2));
				break;
			case 1:
				random2 = (rand() % (medicines.size() + 0));
				hand.push_back(medicines.at(random2));
				break;
			case 2:
				random2 = (rand() % (virus.size() + 0));
				hand.push_back(virus.at(random2));
				break;
			case 3:
				random2 = (rand() % (threatment.size() + 0));
				hand.push_back(threatment.at(random2));
				break;
		}
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

void SendMessages(std::vector<TcpSocket*>* _socks) {
	receiveCards(3);
	
	while (!end) {
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
	while (!end) {
		Status status;
		InputMemoryStream* in = _sock->Receive(status);
		if (status == Status::DISCONNECTED)
		{
			delete in;
			return;
		}

		std::string msg = in->ReadString();
		std::cout << msg << std::endl;
		if (msg == "e" || status != Status::DONE) {
			std::cout << "Socket with ip: " << _sock->GetRemoteAddress() << " and port: " << _sock->GetLocalPort() << " was disconnected" << std::endl;
			for (auto it = _socks->begin(); it != _socks->end(); it++) {
				if (*it == _sock) {
					_socks->erase(it);
					delete _sock;
					delete in;
					return;
				}
			}
		}

		delete in;
	}

}

void AcceptPeers(std::vector<TcpSocket*>* _socks) {
	TcpListener listener;
	listener.Listen(localPort);

	while (_socks->size() < 3 && !end)
	{
		TcpSocket* sock = new TcpSocket();
		Status status = listener.Accept(*sock);
		if (status == Status::DONE) {
			_socks->push_back(sock);
			std::cout << "Connected with ip: " << sock->GetRemoteAddress() << " and port: " << sock->GetLocalPort() << std::endl;

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

		OutputMemoryStream* out = new OutputMemoryStream();
		out->Write(option);

		serverSock.Send(out, status);
		delete out;
		out = new OutputMemoryStream();

		if (option == 3)
		{
			std::cout << "Type server number" << std::endl;
			int server;
			std::cin >> server;

			out->Write(server);
			serverSock.Send(out, status);
			delete out;
			out = new OutputMemoryStream();

			InputMemoryStream* in;
			
			in = serverSock.Receive(status);

			std::string msg = in->ReadString();

			std::cout << msg << std::endl;

			if (msg != "")
			{
				do
				{
					std::cin >> msg;

					out->WriteString(msg);
					serverSock.Send(out, status);

					in = serverSock.Receive(status);

					msg = in->ReadString();
					std::cout << msg << std::endl;

				} while (msg == "Incorrect password. Try again or write 'exit' to leave");

			}

			delete in;
			delete out;
		}
		else if (option == 2)
		{
			InputMemoryStream* in = serverSock.Receive(status);
			int size;
			in->Read(&size);
			for (int i = 0; i < size; i++)
			{
				int idx;
				in->Read(&idx);
				int numOfPlayers;
				in->Read(&numOfPlayers);
				std::cout << "Game number: " << idx << ", Players connected: " << numOfPlayers << std::endl;
			}

			delete in;
		}
		else if (option == 1)
		{
			InputMemoryStream* in = serverSock.Receive(status);
			if (status == Status::DONE)
			{
				std::string msg = in->ReadString();
				std::cout << msg << std::endl;

				std::cin >> msg;
				out->WriteString(msg);

				serverSock.Send(out, status);
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
		organCard->cardType = Card::CardType::ORGAN;
		organCard->organType = (Card::OrganType)(int)i;
		organs.push_back(organCard);

		medicineCard = new Card();
		medicineCard->cardType = Card::CardType::MEDICINE;
		medicineCard->organType = (Card::OrganType)(int)i;
		medicines.push_back(medicineCard);

		virusCard = new Card();
		virusCard->cardType = Card::CardType::VIRUS;
		virusCard->organType = (Card::OrganType)(int)i;
		virus.push_back(medicineCard);
	}

	for (size_t i = 0; i < 6; i++)
	{
		Card* threatmentCard = new Card();
		threatmentCard = new Card();
		threatmentCard->cardType = Card::CardType::TREATMENT;
		threatmentCard->treatmentType = (Card::TreatmentType)(int)i;
		threatment.push_back(threatmentCard);
	}
}

int main() {

	std::vector<TcpSocket*> socks;
	InitializeCards();
	ConnectPeer2Peer(&socks);

	while(!end){}

	return 0;
}