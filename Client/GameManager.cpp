#include "GameManager.h"
#include "SceneManager.h"
#include <thread>

void GameManager::ClientControl(TcpSocket* serverSock)
{
	Selector selector;
	Status status;
	
	selector.Add(serverSock);

	while (true)
	{
		int maxSize = *gameMaxSize;
		if (socks->size() >= maxSize)
		{
			selector.Remove(&listener);
			listener.Close();
		}

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
				OutputMemoryStream* out = nullptr;

				int instruction;
				in->Read(&instruction);

				//----------------------------// CREATE GAME //----------------------------//

				//Adptar el connect game que hagi fet el Mateu	|| Check if the name is the same as one of the games name, if it is return INCORRECT_NAME
				if (instruction == (int)Commands::INCORRECT_NAME)
				{
					out = new OutputMemoryStream();
					out->Write((int)Commands::CREATE_GAME);
					CreateGame(out);
				}
				//----------------------------// SEARCH GAME //----------------------------//

				else if (instruction == (int)Commands::GAME_LIST)
				{
					ListCurrentGames(in);
				}

				//--------------------------// JOIN GAME LOGIC //--------------------------//
				// JOIN GAME
				else if (instruction == (int)Commands::PROTECTED)
				{
					out = new OutputMemoryStream();
					out->Write((int)Commands::PWD_CHECK);
					SendPassword(out);
				}
				else if (instruction == (int)Commands::INCORRECT_ID)
				{
					out = new OutputMemoryStream();
					bool aborted = false;
					JoinGame(out, aborted);
					
					if (aborted)
					{
						delete in;
						delete out;

						continue;
					}
				}
				else if (instruction == (int)Commands::INCORRECT_PWD)
				{
					out = new OutputMemoryStream();
					out->Write((int)Commands::PWD_CHECK);
					SendPassword(out);
				}
				//------------------------// END JOIN GAME LOGIC //------------------------//
				else if (instruction == (int)Commands::PLAYER_LIST)
				{
					out = new OutputMemoryStream();
					selector.Remove(serverSock);
					serverSock->Disconnect();
					ConnectP2P(&selector, in);
					SetListener();
					selector.Add(&listener);

					delete in;
					delete out;

					continue;
				}
				delete in;

				if (out != nullptr)
				{
					serverSock->Send(out, status);
					delete out;
				}
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

							if (socks->size() == 1)
							{
								delete socks->at(0);
								socks->clear();
								*end = true;
								return;
							}

							_checkedIds.resize(socks->size());

							int position = player->id - 1;
							_checkedIds.erase(_checkedIds.begin() + position);

							for (size_t j = 0; j < _checkedIds.size(); j++)
							{
								_checkedIds[j] = j + 1;
							}
							delete socks->at(i);
							socks->erase(socks->begin() + i);


							DisconnectPlayer(i);
							
							continue;
						}
						OutputMemoryStream* out = nullptr;

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
						else if (instruction == (int)Commands::PLAYER_ID)
						{
							int _id;
							in->Read(&_id);

							int position = _id - 1;
							_checkedIds.erase(_checkedIds.begin() + position);
						}
						//Ready
						else if (instruction == (int)Commands::PLAYER_READY)
						{
							bool isReady;
							in->Read(&isReady);

							if (isReady)
							{
								int* newPlayersReady = new int(*playersReady + 1);
								delete playersReady;
								playersReady = newPlayersReady;

								std::cout << "Another player is ready" << std::endl;
							}
						}
						//Log
						else if (instruction == (int)Commands::LOG)
						{
							std::string msg = in->ReadString();

							std::cout << "___________________LOG___________________" << std::endl;
							std::cout << msg << std::endl;
							std::cout << "___________________LOG___________________" << std::endl;
						}
						else if (instruction == (int)Commands::I_WON)
						{
							int _playerID;
							in->Read(&_playerID);
							std::cout << "___________________LOG___________________" << std::endl;
							std::cout << "Player " << _playerID << " won!! ----LOOOOSER----" << std::endl;
							std::cout << "___________________LOG___________________" << std::endl;
							*end = true;
							return;
						}
						//Card logic
						else if (instruction == (int)Commands::PLACE_INFECTION)
						{
							int playerID;
							in->Read(&playerID);

							if (playerID == player->id)
							{
								int organType;
								in->Read(&organType);

								for (size_t _player = 0; _player < table->table.size(); _player++)
								{
									for (size_t cardID = 0; cardID < table->table[_player].size(); cardID++)
									{
										if (table->table.at(_player).at(cardID)->organType == (Card::OrganType)organType || organType == (int)Card::OrganType::NONE)
										{
											if (table->table.at(_player).at(cardID)->VaccineQuantity < 2)
											{
												if (table->table.at(_player).at(cardID)->VaccineQuantity <= 0)
													table->table.at(_player).at(cardID)->virusQuantity += 1;
												else
													table->table.at(_player).at(cardID)->VaccineQuantity -= 1;

												std::cout << "Virus Received!!" << std::endl;

												if (table->table.at(_player).at(cardID)->virusQuantity >= 2)
												{
													Card* _card = table->table[_player][cardID];
													deck->PushCard(_card);

													table->table.at(_player).erase(table->table.at(_player).begin() + cardID);
													sendMSG("Organ destroyed from player: " + std::to_string(player->id));
													std::cout << "Organ Destroyed!!!!!" << std::endl;
												}
												break;
											}
										}
									}
								}
							}
						}
						else if (instruction == (int)Commands::LIST_CARDS)
						{
							out = new OutputMemoryStream();
							int playerID;
							in->Read(&playerID);

							out->Write((int)Commands::SHOW_CARDS);

							out->Write(player->id);

							for (size_t i = 0; i < table->table.size(); i++)
							{
								if (table->table[i].size() > 0)
								{
									int size = table->table.at(i).size();
									out->Write(size);
									break;
								}
							}

							for (size_t i = 0; i < table->table.size(); i++)
							{
								if (table->table[i].size() > 0)
								{
									for (size_t o = 0; o < table->table.at(i).size(); o++)
									{
										out->Write((int)table->table[i][o]->cardType);
										out->Write((int)table->table[i][o]->organType);
										out->Write((int)table->table[i][o]->virusQuantity);
										out->Write((int)table->table[i][o]->VaccineQuantity);
									}
								}
							}
						}
						else if (instruction == (int)Commands::SHOW_CARDS)
						{
							int playerID;
							in->Read(&playerID);
							int tableSize;
							in->Read(&tableSize);
							for (size_t i = 0; i < tableSize; i++)
							{
								int cardType;
								in->Read(&cardType);

								int organType;
								in->Read(&organType);

								int virusQuantity;
								in->Read(&virusQuantity);

								int vaccineQuantity;
								in->Read(&vaccineQuantity);

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
							in->Read(&playerID);
							int threatmentType;
							in->Read(&threatmentType);

							if (playerID == player->id || playerID == (int)Commands::SEND_TO_ALL_PLAYERS)
							{
								int virusQuantity;
								in->Read(&virusQuantity);

								switch ((Card::TreatmentType)threatmentType)
								{
								case (Card::TreatmentType::INFECTION):
									for (size_t i = 0; i < table->table.size(); i++)
									{
										for (size_t o = 0; o < table->table.at(i).size(); o++)
										{
											if (table->table.at(i).at(o)->VaccineQuantity < 2)
											{
												std::cout << "Someone has sent you their viruses!!!!!" << std::endl;
												while (virusQuantity > 0)
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
												if (table->table.at(i).at(o)->virusQuantity >= 2)
												{
													table->table.at(i).erase(table->table.at(i).begin() + o);
													sendMSG("Organ destroyed from player: " + std::to_string(player->id));
													std::cout << "Organ Destroyed!!!!!" << std::endl;
												}

												break;
											}
										}
									}
									break;
								case (Card::TreatmentType::LATEX_GLOVES):
									player->hand.hand.clear();
									std::cout << "Your Hand Was Erased!!!!!" << std::endl;
									break;
								case (Card::TreatmentType::MEDICAL_ERROR):

									break;
								case (Card::TreatmentType::ROBER):
									int playerToSend;
									in->Read(&playerToSend);

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
						//---------------------------// END GAMELOOP //----------------------------//

						if (out != nullptr)
						{
							socks->at(i)->Send(out, status);
							delete in;
						}
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
		playerTurnOrder.push_back(Pair_Organ_Player(player->id, organQuantity));
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

	OutputMemoryStream* out = new OutputMemoryStream();
	//instruction 0: receive the organ quantity to receive the turn
	out->Write((int)Commands::ORGAN_QUANTITY);
	out->Write(player->id);
	out->Write(organQuantity);

	Status status;
	for (size_t i = 0; i < socks->size(); i++)
	{
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

	for (auto it = socks->begin(); it != socks->end(); ++it)
	{
		Status status;
		TcpSocket& client = **it;
		client.Send(out, status);
	}

	delete out;
}

GameManager::GameManager()
{
	
}

GameManager::~GameManager()
{
	for (size_t i = 0; i < socks->size(); i++)
	{
		delete socks->at(i);
	}
	delete socks;

	delete player;
	delete deck;
	delete table;

	delete endRound;
	delete currentTurn;
}

void GameManager::sendMSG(std::string message)
{
	OutputMemoryStream outMSG;

	outMSG.Write((int)Commands::LOG);
	outMSG.WriteString(message);

	for (auto it = socks->begin(); it != socks->end(); ++it)
	{
		Status status;
		TcpSocket& client = **it;
		client.Send(&outMSG, status);
	}
}

void GameManager::ListEnemiesWithTheirCards() 
{
	OutputMemoryStream* out = new OutputMemoryStream();

	out->Write((int)Commands::LIST_CARDS);
	out->Write(player->id);

	Status status;

	for (auto it = socks->begin(); it != socks->end(); ++it)
	{
		Status status;
		TcpSocket& client = **it;
		client.Send(out, status);
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

	sendMSG("Payer: " + std::to_string(player->id) + " has just infected an organ from the player " + std::to_string(objective));

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
				sendMSG("Payer: " + std::to_string(player->id) + " has just vaccined an organ");
				if(table->table[i][o]->VaccineQuantity >= 2)
				{
					std::cout << "your organ has become invincible!!!!" << std::endl;
					sendMSG("An organ from the player " + std::to_string(player->id) + " had become invincible!");
				}
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

		int tableIndex = -1;

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
		out.Write((int)Commands::SEND_TO_ALL_PLAYERS);
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
	if (*end) return true;

	for (size_t i = 0; i < table->table.size(); i++)
	{
		if (table->table[i].size() >= 4)
		{
			std::cout << "I won!!!!" << std::endl;
			OutputMemoryStream* out = new OutputMemoryStream();
			out->Write((int)Commands::I_WON);
			out->Write(player->id);
			Status status;
			for (size_t j = 0; j < socks->size(); j++)
			{
				socks->at(j)->Send(out, status);
			}
			delete out;
			*end = true;
		}
	}

	//Check if a player quit
	if (!_checkedIds.empty())
	{
		if (_checkedIds.size() == 1)
		{
			for (size_t i = 0; i < playerTurnOrder.size(); i++)
			{
				if (playerTurnOrder[i].playerID == _checkedIds.front())
				{
					playerTurnOrder.erase(playerTurnOrder.begin() + i);

					_checkedIds.clear();
					break;
				}
			}
		}
	}

	//Check end round
	if (*currentTurn == playerTurnOrder.size() || playerTurnOrder[*currentTurn].playerID != player->id)
		return *endRound;

	for (size_t i = 0; i < playerTurnOrder.size(); i++)
		std::cout << "Turn player: " << playerTurnOrder[i].playerID << std::endl;

	bool finishedRound = false;

	while (!finishedRound)
	{
		player->ReceiveCards(MAX_CARDS - player->hand.hand.size(), deck);

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

		if (*end) return true;

		Commands option = Commands::COUNT;
		std::string tmpOption;
		std::cin >> tmpOption;

		if (tmpOption == "1")
		{
			option = Commands::PLACE_ORGAN;
		}
		else if (tmpOption == "2")
		{
			option = Commands::PLACE_INFECTION;
		}
		else if (tmpOption == "3")
		{
			option = Commands::PLACE_MEDICINE;
		}
		else if (tmpOption == "4")
		{
			option = Commands::DISCARD_CARD;
		}
		else if (tmpOption == "5")
		{
			option = Commands::PLACE_TREATMENT;
		}

		int card;

		switch (option)
		{
		case Commands::PLACE_ORGAN:
			player->hand.ListCards();
			std::cout << "Choose a card: ('3' to go back)" << std::endl;

			std::cin >> card;

			if(card >= 3)
				finishedRound = false;

			if(card < 3 && player->PlaceCard(card, Card::CardType::ORGAN, table, deck) == true) 
			{
				finishedRound = true;
				std::cout << "Organ Placed!" << std::endl;
				sendMSG("Payer: " + std::to_string(player->id) + " has just placed an organ on the table");
			}

			break;
		case Commands::PLACE_INFECTION:
			if(PlaceInfection()) 
			{
				finishedRound = true;
				break;
			}
			finishedRound = false;
			break;
		case Commands::PLACE_MEDICINE:
			if(VaccineOrgan()) 
			{
				finishedRound = true;
				break;
			}
			finishedRound = false;
			break;
		case Commands::PLACE_TREATMENT:
			if(Threatment()) 
			{
				finishedRound = true;
				break;
			}
			finishedRound = false;
			break;
		case Commands::DISCARD_CARD:

			if(true) 
			{
				int quantity = 0;

				while (true)
				{
					player->hand.ListCards();
					std::cout << "Choose a card: ('3' to accept)" << std::endl;

					std::cin >> card;

					if (card >= 3 || card >= player->hand.hand.size())
						break;

					Card* _card = player->hand.hand[card];
					deck->PushCard(_card);

					player->hand.hand.erase(player->hand.hand.begin() + card);
					std::cout << "Card Removed!" << std::endl;
					sendMSG("Payer: " + std::to_string(player->id) + " discarded one card");
					quantity++;
				}

				if (quantity == 0)
				{
					finishedRound = false;
					break;
				}

				finishedRound = true;
			}
			break;
		default:
			break;
		}
	}

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
	out->Write((int)Commands::PLAYER_READY);
	out->Write(true);

	Status status;

	for (size_t i = 0; i < socks->size(); i++)
	{
		socks->at(i)->Send(out, status);
	}

	delete out;
}

void GameManager::SetListener()
{
	Status status = listener.Listen(localPort);
	if (status != Status::DONE)
		return;
}

void GameManager::DisconnectPlayer(int id)
{
	Status status;
	OutputMemoryStream* out = new OutputMemoryStream();

	out->Write((int)Commands::PLAYER_ID);
	out->Write(player->id);

	for (size_t i = 0; i < socks->size(); i++)
	{
		if (i != id)
		{
			socks->at(i)->Send(out, status);
		}
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

void GameManager::CreateGame(OutputMemoryStream* out)
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

	Status status;
	out->WriteString(gameName);
	out->Write(numOfPlayers);
	out->WriteString(gamePassword);
}

void GameManager::ActivateFilters(OutputMemoryStream* out)
{
	Status status;
	Commands filter = Commands::DEFAULT;
	bool wantsPwd;
	int numOfPlayersWanted;
	while (filter == Commands::DEFAULT)
	{
		std::cout << "Do you want to filter by password or number of players? (pwd/num/no) " << std::endl;
		std::string tmpFilter;
		std::cin >> tmpFilter;
		if (tmpFilter == "pwd") {
			filter = Commands::PWD_FILTER;
			while (true) {
				std::cout << "Do you want it to have a password or not? (y/n) " << std::endl;
				std::string wantsPwdAns;
				std::cin >> wantsPwdAns;
				if (wantsPwdAns == "y" || wantsPwdAns == "Y") {
					wantsPwd = true;
					break;
				}
				else if (wantsPwdAns == "n" || wantsPwdAns == "N") {
					wantsPwd = false;
					break;
				}
			}
		}
		else if (tmpFilter == "num") {
			filter = Commands::NUM_PLAYERS_FILTER;
			while (true) {
				std::cout << "What amount of players do you want to seek? " << std::endl;
				int numOfPlayersAns;
				std::cin >> numOfPlayersAns;
				if (numOfPlayersAns >= 2 && numOfPlayersAns <= 4) {
					numOfPlayersWanted = numOfPlayersAns;
					break;
				}
			}
		}
		else if (tmpFilter == "no") filter = Commands::NO_FILTER;
	}
	out->Write((int)filter);
	if (filter == Commands::PWD_FILTER) out->Write(wantsPwd);
	else if (filter == Commands::NUM_PLAYERS_FILTER) out->Write(numOfPlayersWanted);
}


void GameManager::ListCurrentGames(InputMemoryStream* in)
{
	//Get Num of games
	int numOfGames;
	in->Read(&numOfGames);

	std::cout << "Getting List of Games. There are: " << numOfGames << std::endl;

	if (numOfGames == 0) {
		std::cout << "No games found!\n";
	}
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

void GameManager::SendPassword(OutputMemoryStream* out)
{
	int gameID = *currentGameID;
	out->Write(gameID);

	std::cout << "Write the password" << std::endl;
	std::string password;
	std::cin >> password;

	out->WriteString(password);
}

void GameManager::ConnectP2P(Selector* selector, InputMemoryStream* in)
{
	std::cout << "---------------Connect---------------" << std::endl;
	Status status;
	int maxSize;
	in->Read(&maxSize);

	SetGameSize(maxSize);

	int playerNum;
	in->Read(&playerNum);

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
			std::cout << "Error connection with peer" << std::endl;
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
