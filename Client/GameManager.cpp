#include "GameManager.h"
#include "SceneManager.h"
#include <thread>

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

				//Adptar el connect game que hagi fet el Mateu	|| Check if the name is the same as one of the games name, if it is return INCORRECT_NAME
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
					bool aborted = false;
					JoinGame(out, aborted);
					
					if (aborted)
					{
						delete in;
						delete out;

						continue;
					}
				}
				// PWD CHECK
				else if (instruction == (int)Commands::CORRECT_PWD)
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

	OutputMemoryStream* out = new OutputMemoryStream();
	//instruction 0: receive the organ quantity to receive the turn
	out->Write((int)Commands::ORGAN_QUANTITY);
	out->Write(player->id);
	out->Write(organQuantity);

	std::cout << organQuantity << std::endl;

	for (auto it = socks->begin(); it != socks->end(); ++it)
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

	for (auto it = socks->begin(); it != socks->end(); ++it)
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

	for (auto it = socks->begin(); it != socks->end(); ++it)
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

	for (auto it = socks->begin(); it != socks->end(); ++it)
	{
		Status status;
		TcpSocket& client = **it;
		client.Send(out, status);
	}

	player->hand.hand.erase(player->hand.hand.begin() + card);

	return true;

	delete out;
}

bool GameManager::VaccineOrgan()
{
	int card;
	player->hand.ListCards();
	std::cout << "Choose your vaccine card ('3' to go back)" << std::endl;
	std::cin >> card;
	if (card >= 3 || player->hand.hand[card]->cardType != Card::CardType::MEDICINE)
		return false;

	for (size_t i = 0; i < table->table.size(); i++)
	{
		for (size_t o = 0; o < table->table[i].size(); o++)
		{
			if(table->table[i][o]->organType == player->hand.hand[card]->organType || player->hand.hand[card]->organType == Card::OrganType::NONE)
			{
				table->table[i][o]->VaccineQuantity += 1;
				player->hand.hand.erase(player->hand.hand.begin() + card);
				return true;
			}
		}
	}
}

bool GameManager::Threatment() 
{
	int card;
	player->hand.ListCards();
	std::cout << "Choose your threatment card ('3' to go back)" << std::endl;
	std::cin >> card;
	if (card >= 3 || player->hand.hand[card]->cardType != Card::CardType::TREATMENT)
		return false;

	if(player->hand.hand[card]->treatmentType == Card::TreatmentType::INFECTION) 
	{
		int cardI;
		table->ShowTable();
		std::cout << "Choose your infected card ('3' to go back)" << std::endl;
		std::cin >> cardI;

		int tableIndex;

		for (size_t i = 0; i < table->table.size(); i++)
		{
			if (table->table[i].size() > 0 && table->table[i][cardI]->virusQuantity <= 0)
				return false;
			else if (table->table[i].size() > 0) 
			{
				tableIndex = i;
				break;
			}
		}

		std::cout << "Choose an objective ('3' to go back)" << std::endl;

		ListEnemiesWithTheirCards();

		int objective;
		std::cin >> objective;
		if (objective >= 3)
			return false;

		OutputMemoryStream out;

		out.Write((int)Commands::PLACE_TREATMENT);
		out.Write(objective);
		out.Write((int)Card::TreatmentType::INFECTION);
		out.Write(table->table[tableIndex][cardI]->virusQuantity);

		for (auto it = socks->begin(); it != socks->end(); ++it)
		{
			Status status;
			TcpSocket& client = **it;
			client.Send(&out, status);
		}

		table->table[tableIndex][cardI]->virusQuantity = 0;
		player->hand.hand.erase(player->hand.hand.begin() + card);
		return true;

	}
	if (player->hand.hand[card]->treatmentType == Card::TreatmentType::ROBER)
	{
		std::cout << "Choose an objective ('3' to go back)" << std::endl;

		ListEnemiesWithTheirCards();

		int objective;
		std::cin >> objective;
		if (objective >= 3)
			return false;

		OutputMemoryStream out;

		out.Write((int)Commands::PLACE_TREATMENT);
		out.Write(objective);
		out.Write((int)Card::TreatmentType::ROBER);
		out.Write(player->id);
		out.Write(0);

		for (auto it = socks->begin(); it != socks->end(); ++it)
		{
			Status status;
			TcpSocket& client = **it;
			client.Send(&out, status);
		}

		player->hand.hand.erase(player->hand.hand.begin() + card);
		return true;
	}
	else if (player->hand.hand[card]->treatmentType == Card::TreatmentType::TRANSPLANT)
	{
		//organ swap
	}
	else if (player->hand.hand[card]->treatmentType == Card::TreatmentType::LATEX_GLOVES)
	{
		OutputMemoryStream out;

		out.Write((int)Commands::PLACE_TREATMENT);
		out.Write(3);
		out.Write((int)Card::TreatmentType::LATEX_GLOVES);

		for (auto it = socks->begin(); it != socks->end(); ++it)
		{
			Status status;
			TcpSocket& client = **it;
			client.Send(&out, status);
		}

		player->hand.hand.erase(player->hand.hand.begin() + card);
		return true;
	}
	else if (player->hand.hand[card]->treatmentType == Card::TreatmentType::MEDICAL_ERROR)
	{
		//body swap
	}
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
			if(VaccineOrgan() == true) 
			{
				finishedRound = true;
				break;
			}
			finishedRound = false;
			break;
		case Commands::PLACE_TREATMENT:
			if(Threatment() == true) 
			{
				finishedRound = true;
				break;
			}
			finishedRound = false;
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

	for (auto it = socks->begin(); it != socks->end(); ++it)
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
							if(table->table.at(i).at(o)->VaccineQuantity <= 0)
								table->table.at(i).at(o)->virusQuantity += 1;
							else 
								table->table.at(i).at(o)->VaccineQuantity -= 1;

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
						out.Write((int)table->table[i][o]->VaccineQuantity);
					}
				}
			}

			mtx.unlock();

			int index = 0;

			for (auto it = socks->begin(); it != socks->end(); ++it)
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

				int vaccineQuantity;
				in1.Read(&vaccineQuantity);

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

				std::cout << "PLAYER: " << playerID << " | CARD: " << _cardType << " ORGAN: " << _organType << " VIRUS: " << virusQuantity << " VACCINES: " << vaccineQuantity << std::endl;
			}
		}
		else if (instruction == (int)Commands::PLACE_TREATMENT)
		{
			int playerID;
			in1.Read(&playerID);
			int threatmentType;
			in1.Read(&threatmentType);

			if(playerID == player->id || playerID == 3) 
			{
				int virusQuantity;
				in1.Read(&virusQuantity);

				switch ((Card::TreatmentType)threatmentType)
				{
				case (Card::TreatmentType::INFECTION):
					for (size_t i = 0; i < table->table.size(); i++)
					{
						for (size_t o = 0; o < table->table.at(i).size(); o++)
						{
							std::cout << "Someone has sent you their viruses!!!!!" << std::endl;
							while(virusQuantity > 0) 
							{
								if (table->table.at(i).at(o)->VaccineQuantity <= 0) 
								{
									table->table.at(i).at(o)->virusQuantity += virusQuantity;
									virusQuantity = 0;
								}
								else 
								{
									table->table.at(i).at(o)->VaccineQuantity -= 1;
									virusQuantity -= 1;
								}
							}
							break;
						}
					}
					break;
				case (Card::TreatmentType::LATEX_GLOVES):
					player->hand.hand.clear();
					std::cout << "Your Hand Was Erased!!!!!" << std::endl;
					break;
				case (Card::TreatmentType::MEDICAL_ERROR):

					break;
				case (Card::TreatmentType::NONE):

					break;
				case (Card::TreatmentType::ROBER):
					int playerToSend;
					in1.Read(&playerToSend);
					std::cout << "player: " << playerToSend << std::endl;
					for (size_t i = 0; i < table->table.size(); i++)
					{
						for (size_t o = 0; o < table->table[i].size(); o++)
						{
							table->table[i].erase(table->table[i].begin() + o);
							break;
						}
					}
					break;
				case (Card::TreatmentType::TRANSPLANT):

					break;
				}
			}
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

void GameManager::JoinGame(OutputMemoryStream* out, bool& _aborted)
{
	//Choose game
	std::cout << "Type server ID" << std::endl;
	char tmpOption;
	std::cin >> tmpOption;
	int serverIdx = tmpOption - '0';

	if (serverIdx < 0)
	{
		_aborted = true;
		return;
	}

	*currentGameID = serverIdx;

	out->Write(serverIdx);
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

void GameManager::SendPassword(OutputMemoryStream* out)
{
	int gameID = *currentGameID;
	out->Write(gameID);

	std::string password;
	std::cin >> password;

	out->WriteString(password);

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
