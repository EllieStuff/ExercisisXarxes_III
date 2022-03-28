#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include "..\res\Selector.h"
#include "..\res\TcpSocket.h"
#include "..\res\TcpListener.h"

class OutputMemoryStream;
class InputMemoryStream;
class TcpSocket;
class Selector;
class TcpListener;

void ConnectToServer(std::vector<Game>* _games, TcpSocket* sock, int _gameID, OutputMemoryStream* out)
{
	out->Write((int)Commands::PLAYER_LIST);
	Status status;
	
	for (size_t i = 0; i < _games->size(); i++)
	{
		if (_games->at(i).gameId == _gameID)
		{	
			std::cout << "Connected with " << sock->GetRemoteAddress() << ". Curr Size = " << _games->at(i).peers.size() << std::endl;
			out->Write(_games->at(i).gameMaxSize);
			out->Write((int)_games->at(i).peers.size());
			for (size_t j = 0; j < _games->at(i).peers.size(); j++)
			{
				PeerAddress current = _games->at(i).peers[j];
				std::cout << _games->at(i).peers[j].ip << ", " << _games->at(i).peers[j].port << std::endl;
				out->WriteString(current.ip);
				out->Write(current.port);
			}

			if (_games->at(i).peers.size() < _games->at(i).gameMaxSize)
			{
				PeerAddress address;
				address.ip = sock->GetRemoteAddress();
				address.port = sock->GetRemotePort();
				_games->at(i).peers.push_back(address);

			}
			else
				_games->erase(_games->begin() + i);

			break;
		}
	}
}

void SendGameList(std::vector<Game>* _gamesToSend, OutputMemoryStream* out)
{
	out->Write((int)Commands::GAME_LIST);
	out->Write((int)_gamesToSend->size());

	for (size_t i = 0; i < _gamesToSend->size(); i++)
	{
		out->Write(_gamesToSend->at(i).gameId);
		out->WriteString(_gamesToSend->at(i).gameName);
		out->Write(_gamesToSend->at(i).gameMaxSize);
		out->Write((int)_gamesToSend->at(i).peers.size());
	}
}

void ServerControl(std::vector<Game>* _games)
{
	int currGameId = 0;
	std::vector<TcpSocket*> socks;

	Selector selector;

	TcpListener listener;
	Status status = listener.Listen(50000);
	if (status != Status::DONE)
		return;

	selector.Add(&listener);

	while (true)
	{
		for (size_t i = 0; i < _games->size(); i++)
		{
			if (_games->at(i).peers.size() == 0)
			{
				_games->erase(_games->begin() + i);
				i--;
			}
		}

		if (selector.Wait())
		{
			if (selector.IsReady(&listener))
			{
				TcpSocket* sock = new TcpSocket();
				status = listener.Accept(*sock);
				if (status != Status::DONE) {
					std::cout << "Error client connection: " << sock->GetRemoteAddress() << std::endl;
					delete sock;
					continue;
				}
				socks.push_back(sock);
				selector.Add(sock);
				std::cout << "Client connected: " << sock->GetRemoteAddress() << std::endl;
			}
			else
			{
				for (size_t i = 0; i < socks.size(); i++)
				{
					if (selector.IsReady(socks[i]))
					{
						Status status;
						InputMemoryStream* in = socks[i]->Receive(status);
						if (status != Status::DONE)
						{
							delete in;
							selector.Remove(socks[i]);
							continue;
						}

						int menuOption;

						in->Read(&menuOption);
						OutputMemoryStream* out = nullptr;

						//--------------------------// CREATE GAME LOGIC //--------------------------//
						if (menuOption == (int)Commands::CREATE_GAME)			//CREATE GAME
						{
							out = new OutputMemoryStream();
							std::cout << "Create game asked. Num on games: " << _games->size() << std::endl;
							Game game;
							game.gameId = currGameId;

							//Game Name
							std::string gameName = in->ReadString();
							game.gameName = gameName;

							//Comprovar contrasenya correcta

							//Num players
							int numPlayers;
							in->Read(&numPlayers);
							game.gameMaxSize = numPlayers;

							//Password
							std::string password = in->ReadString();
							game.pwd = password;

							_games->push_back(game);
							int size = _games->size() - 1;

							ConnectToServer(_games, socks[i], game.gameId, out);
							currGameId++;
						}

						//--------------------------// END CREATE GAME LOGIC //--------------------------//
						else if (menuOption == (int)Commands::GAME_LIST)
						{
							out = new OutputMemoryStream();
							int filter;
							in->Read(&filter);

							bool wantsPwd;
							int numOfPlayersWanted;

							if (filter == (int)Commands::PWD_FILTER) in->Read(&wantsPwd);
							else if (filter == (int)Commands::NUM_PLAYERS_FILTER) in->Read(&numOfPlayersWanted);
							else if (filter == (int)Commands::NO_FILTER);

							int actualSize = _games->size();
							if (filter != (int)Commands::NO_FILTER) {
								std::vector<Game> _filterGames;
								actualSize = 0;
								for (size_t i = 0; i < _games->size(); i++)
								{
									Game currGame = _games->at(i);
									if (filter == (int)Commands::PWD_FILTER) {
										if (wantsPwd && currGame.pwd != "")
										{
											_filterGames.push_back(_games->at(i));
											actualSize++;
										}
										else if (!wantsPwd && currGame.pwd == "")
										{
											_filterGames.push_back(_games->at(i));
											actualSize++;
										}
									}
									else if (filter == (int)Commands::NUM_PLAYERS_FILTER)
									{
										if (numOfPlayersWanted == currGame.gameMaxSize)
										{
											_filterGames.push_back(_games->at(i));
											actualSize++;
										}
									}
								}
								std::cout << "Sending filtered lists" << std::endl;
								SendGameList(&_filterGames, out);
							}
							else
								SendGameList(_games, out);
						}

						//--------------------------// JOIN GAME LOGIC //--------------------------//
						else if (menuOption == (int)Commands::JOIN_GAME)		//JOIN GAME
						{
							out = new OutputMemoryStream();
							std::cout << "Join game asked" << std::endl;
							Game* game = nullptr;
							//Get game id
							int gameID;

							in->Read(&gameID);
							bool valid = false;
							for (size_t i = 0; i < _games->size(); i++)
							{
								if (gameID == _games->at(i).gameId)
								{
									game = &_games->at(i);
									valid = true;
									break;
								}
							}

							if (valid && game != nullptr)
							{
								if (game->pwd.compare("") > 0)
								{
									std::cout << "Game protected found" << std::endl;
									// PROTECTED
									out->Write((int)Commands::PROTECTED);
								}
								else
								{
									std::cout << "Game not protected found" << std::endl;
									//NOT_PROTECTED
									ConnectToServer(_games, socks[i], gameID, out);
								}
							}
							else
							{
								//INCORRECT_ID
								out->Write((int)Commands::INCORRECT_ID);
							}
						}
						else if (menuOption == (int)Commands::PWD_CHECK)			//GAME PWD CHECK
						{
							out = new OutputMemoryStream();
							//Get game id
							int gameID;

							in->Read(&gameID);
							std::string password = in->ReadString();
							bool valid = false;

							for (size_t i = 0; i < _games->size(); i++)
							{
								if (gameID == _games->at(i).gameId && password.compare(_games->at(i).pwd.c_str()) == 0)
								{
									valid = true;
								}
							}

							if (valid)
							{
								// CORRECT_PWD
								ConnectToServer(_games, socks[i], gameID, out);
							}
							else
							{
								// INCORRECT_PWD
								out->Write((int)Commands::INCORRECT_PWD);
							}

						}
						//--------------------------// END JOIN GAME LOGIC //--------------------------//
						delete in;
						if (out != nullptr)
						{
							socks[i]->Send(out, status);
							delete out;
						}
					}
				}
			}
		}
	}

	listener.Close();
}

int main() {
	std::vector<Game> games;

	ServerControl(&games);

	return 0;
}