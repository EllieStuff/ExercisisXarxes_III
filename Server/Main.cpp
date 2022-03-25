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
	Status status;
	std::cout << "Connected with " << sock->GetRemoteAddress() << ". Curr Size = " << _games->at(_gameID).peers.size() << std::endl;
	out->Write(_games->at(_gameID).peers.size());

	for (size_t i = 0; i < _games->size(); i++)
	{
		if (_games->at(i).gameId == _gameID)
		{
			for (size_t j = 0; j < _games->at(i).peers.size(); j++)
			{
				PeerAddress current = _games->at(_gameID).peers[j];
				std::cout << _games->at(_gameID).peers[j].ip << ", " << _games->at(_gameID).peers[j].port << std::endl;
				out->WriteString(current.ip);
				out->Write(current.port);
			}

			if (_games->at(i).peers.size() < 3)
			{
				PeerAddress address;
				address.ip = sock->GetRemoteAddress();
				address.port = sock->GetRemotePort();

				_games->at(i).peers.push_back(address);

			}
			else
			{
				_games->erase(_games->begin() + i);
			}
		}
	}

	sock->Disconnect();
}

//void ClientMenu(TcpSocket* sock, std::vector<Game>* peerAddresses)
//{
//	std::cout << "Connected with " << sock->GetRemoteAddress() << std::endl;
//	//sending menu int;
//
//	while (true)
//	{
//		Status status;
//		InputMemoryStream* in = sock->Receive(status);
//		if (status != Status::DONE)
//			continue;
//
//		int menuOption;
//
//		in->Read(&menuOption);
//		delete in;
//
//		//create game
//		if (menuOption == (int)Commands::CREATE_GAME)
//		{
//			//----------
//			mtx.lock();
//			peerAddresses->push_back(Game());
//			int size = peerAddresses->size() - 1;
//			std::cout << peerAddresses->size() << std::endl;
//			mtx.unlock();
//			//----------
//
//			std::string msg = "";
//
//			in = sock->Receive(status);
//
//			if(status == Status::DONE) 
//			{
//				msg = in->ReadString();
//
//				if (msg._Equal("-")) msg = "";
//
//				//----------
//				mtx.lock();
//
//				std::cout << size << std::endl;
//				std::cout << peerAddresses->size() << std::endl;
//
//				peerAddresses->at(size).gameId = currGameId;
//
//				peerAddresses->at(size).pwd = msg;
//				//mtx.unlock();
//				////----------
//
//				ConnectToServer(peerAddresses, sock, size, out);
//				
//				////----------
//				//mtx.lock();
//				currGameId++;
//				mtx.unlock();
//				//----------
//
//				break;
//			}
//		}
//		//search game
//		else if (menuOption == (int)Commands::GAME_LIST)
//		{
//			//----------
//			mtx.lock();
//			OutputMemoryStream* out = new OutputMemoryStream();
//			out->Write((int)peerAddresses->size());
//
//			for (size_t i = 0; i < peerAddresses->size(); i++)
//			{
//				out->Write(peerAddresses->at(i).gameId);
//				out->Write((int)peerAddresses->at(i).peers.size());
//			}
//			mtx.unlock();
//			//----------
//			sock->Send(out, status);
//			delete out;
//		}
//		//connect
//		else if (menuOption == (int)Commands::JOIN_GAME)
//		{
//			if(status == Status::DONE) 
//			{
//				OutputMemoryStream* out;
//				bool validIdx;
//				int serverIndex = false;
//				do {
//					in = sock->Receive(status);
//					in->Read(&serverIndex);
//					delete in;
//
//					//----------
//					mtx.lock();
//					validIdx = serverIndex >= 0 && serverIndex < peerAddresses->size();
//					mtx.unlock();
//					//----------
//
//					out = new OutputMemoryStream();
//					out->Write(validIdx);
//					sock->Send(out, status);
//					delete out;
//				} while (!validIdx);
//
//				//std::string msg = "";
//				out = new OutputMemoryStream();
//
//				//----------
//				mtx.lock();
//				if (peerAddresses->at(serverIndex).pwd != "")
//					out->WriteString("Server protected with password");
//				else
//					out->WriteString("");
//				mtx.unlock();
//				//----------
//
//				sock->Send(out, status);
//				delete out;
//
//				//----------
//				mtx.lock();
//				bool validPassword = peerAddresses->at(serverIndex).pwd == "";
//				mtx.unlock();
//				//----------
//				while (!validPassword)
//				{
//					in = sock->Receive(status);
//
//					if(status == Status::DONE) 
//					{
//						std::string msg = in->ReadString();
//						delete in;
//
//						if (status != Status::DONE || msg == "exit")
//						{
//							break;
//						}
//
//						//----------
//						mtx.lock();
//						validPassword = msg == peerAddresses->at(serverIndex).pwd;
//						mtx.unlock();
//						//----------
//
//						out = new OutputMemoryStream();
//						out->Write(validPassword);
//						sock->Send(out, status);
//						delete out;
//
//					}
//				}
//
//				std::cout << "connect! " << peerAddresses->size() - 1 << " " << serverIndex << std::endl;
//
//				if (peerAddresses->size() - 1 >= serverIndex)
//				{
//					//----------
//					mtx.lock();
//					ConnectToServer(peerAddresses, sock, serverIndex, out);
//					mtx.unlock();
//					//----------
//				}
//				break;
//			}
//		}
//		else
//		{
//			/*OutputMemoryStream* out = new OutputMemoryStream();
//			out->WriteString("");
//
//			sock->Send(out, status);*/
//		}
//	};
//}

void ServerControl(std::vector<Game>* _games)
{
	int currGameId = 0;
	std::vector<TcpSocket*> socks;

	Selector selector;

	TcpListener listener;
	Status status = listener.Listen(50000);
	if (status != Status::DONE) {
		return;
	}
	selector.Add(&listener);

	while (true)
	{
		if (selector.Wait())
		{
			if (selector.IsReady(&listener))
			{
				TcpSocket* sock = new TcpSocket();
				status = listener.Accept(*sock);
				if (status != Status::DONE) {
					delete sock;
					continue;
				}
				socks.push_back(sock);
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
							selector.Remove(socks[i]);
							continue;
						}

						int menuOption;

						in->Read(&menuOption);
						OutputMemoryStream* out = new OutputMemoryStream();

						//--------------------------// CREATE GAME LOGIC //--------------------------//
						if (menuOption == (int)Commands::CREATE_GAME)			//CREATE GAME
						{
							Game game;
							game.gameId = currGameId;

							//Game Name
							std::string gameName = in->ReadString();
							game.gameName = gameName;
							
							//Num players
							int numPlayers;
							in->Read(&numPlayers);
							game.gameSize = numPlayers;

							//Password
							std::string password = in->ReadString();
							game.pwd = password;

							_games->push_back(game);
							int size = _games->size() - 1;
							std::cout << _games->size() << std::endl;

							ConnectToServer(_games, socks[i], game.gameId, out);
							currGameId++;
						}

						//--------------------------// END CREATE GAME LOGIC //--------------------------//
						else if (menuOption == (int)Commands::GAME_LIST)		//SEARCH GAME
						{
							out->Write(menuOption);
							out->Write((int)_games->size());

							for (size_t i = 0; i < _games->size(); i++)
							{
								out->Write(_games->at(i).gameId);
								out->Write((int)_games->at(i).peers.size());
							}
						}

						//--------------------------// JOIN GAME LOGIC //--------------------------//
						else if (menuOption == (int)Commands::JOIN_GAME)		//JOIN GAME
						{
							Game* game;
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

							if (valid)
							{
								if (game->pwd.compare("") > 0)
								{
									// PROTECTED
									out->Write((int)Commands::PROTECTED);
								}
								else
								{
									//NOT_PROTECTED
									out->Write((int)Commands::NOT_PROTECTED);
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
							//Get game id
							int gameID;

							in->Read(&gameID);
							std::string msg = in->ReadString();
							bool valid = false;

							for (size_t i = 0; i < _games->size(); i++)
							{
								if (gameID == _games->at(i).gameId && msg.compare(_games->at(i).pwd.c_str()) == 0)
								{
									valid = true;
								}
							}

							if (valid)
							{
								// CORRECT_PWD
								out->Write((int)Commands::CORRECT_PWD);
								ConnectToServer(_games, socks[i], gameID, out);
							}
							else
							{
								// INCORRECT_PWD
								out->Write((int)Commands::INCORRECT_PWD);
							}
							
						}
						//--------------------------// END JOIN GAME LOGIC //--------------------------//

						socks[i]->Send(out, status);
					}
				}
			}
		}
	}

	listener.Close();
}

int main() {
	std::vector<Game> games;

	std::thread tServer(ServerControl, &games);
	tServer.detach();

	while (true)
	{
		for (size_t i = 0; i < games.size(); i++)
		{
			if (games[i].peers.size() == 0)
			{
				games.erase(games.begin() + i);
				i--;
			}
		}
	}

	return 0;
}