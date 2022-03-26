#include <iostream>
#include <vector>
#include <thread>
#include <time.h>
#include <mutex>
#include "..\res\TcpSocket.h"
#include "..\res\TcpListener.h"



class OutputMemoryStream;
class InputMemoryStream;
class TcpSocket;
class TcpListener;

std::vector<clock_t> peerTimers;
int closeTime = 120 * CLOCKS_PER_SEC;
std::mutex mtx;
int currGameId = 0;

void ConnectToServer(std::vector<Game>* peerAddresses, TcpSocket* sock, int serverIndex)
{
	Status status;
	std::cout << "Connected with " << sock->GetRemoteAddress() << ". Curr Size = " << peerAddresses->at(serverIndex).peers.size() << std::endl;
	OutputMemoryStream out;
	out.Write(peerAddresses->at(serverIndex).peers.size());
	for (int i = 0; i < peerAddresses->at(serverIndex).peers.size(); i++) {
		PeerAddress current = peerAddresses->at(serverIndex).peers[i];
		std::cout << peerAddresses->at(serverIndex).peers[i].ip << ", " << peerAddresses->at(serverIndex).peers[i].port << std::endl;
		out.WriteString(current.ip);
		out.Write(current.port);
	}
	sock->Send(&out, status);
	std::cout << (int)status << std::endl;

	if (peerAddresses->at(serverIndex).peers.size() < peerAddresses->at(serverIndex).gameMaxSize)
	{
		mtx.lock();
		PeerAddress address;
		address.ip = sock->GetRemoteAddress();
		address.port = sock->GetRemotePort();
		peerAddresses->at(serverIndex).peers.push_back(address);
		peerTimers[serverIndex] = clock() + closeTime;

		if (peerAddresses->at(serverIndex).peers.size() >= peerAddresses->at(serverIndex).gameMaxSize) 
		{
			peerAddresses->erase(peerAddresses->begin() + serverIndex);
			peerTimers.erase(peerTimers.begin() + serverIndex);
		}

		mtx.unlock();

	}
	/*else
	{
		peerAddresses->erase(peerAddresses->begin() + serverIndex);
		peerTimers.erase(peerTimers.begin() + serverIndex);
	}*/

	sock->Disconnect();
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
							delete in;
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
								out->Write(_games->at(i).gameName);
								out->Write(_games->at(i).gameMaxSize);
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

						delete in;
						socks[i]->Send(out, status);
					}
				}
			}
		}
	}

	listener.Close();
}


void ClientMenu(TcpSocket* sock, std::vector<Game>* peerAddresses)
{
	std::cout << "Connected with " << sock->GetRemoteAddress() << std::endl;
	//sending menu int;

	while (true)
	{
		Status status;
		InputMemoryStream* in = sock->Receive(status);
		if (status != Status::DONE)
			continue;

		mtx.lock();
		int menuOption;

		in->Read(&menuOption);
		delete in;
		mtx.unlock();

		//create game
		if (menuOption == (int)Commands::CREATE_GAME)
		{
			mtx.lock();
			peerAddresses->push_back(Game());
			int size = peerAddresses->size() - 1;
			std::cout << peerAddresses->size() << std::endl;
			mtx.unlock();

			std::string msg = "";

			in = sock->Receive(status);

			if(status == Status::DONE) 
			{
				mtx.lock();
				int command, numOfPlayers;
				std::string gameName, gamePassword;
				in->Read(&command);
				gameName = in->ReadString();
				in->Read(&numOfPlayers);
				gamePassword = in->ReadString();
				delete in;

				//if (msg._Equal("-")) msg = "";

				peerTimers.push_back(clock() + closeTime);
				mtx.unlock();

				std::cout << size << std::endl;
				std::cout << peerAddresses->size() << std::endl;

				peerAddresses->at(size).gameId = currGameId;
				peerAddresses->at(size).gameName = gameName;
				peerAddresses->at(size).gameMaxSize = numOfPlayers;
				peerAddresses->at(size).pwd = gamePassword;

				ConnectToServer(peerAddresses, sock, size);

				currGameId++;

				break;
			}
		}
		//search game
		else if (menuOption == (int)Commands::GAME_LIST)
		{
			OutputMemoryStream* out = new OutputMemoryStream();
			out->Write((int)peerAddresses->size());

			for (size_t i = 0; i < peerAddresses->size(); i++)
			{
				out->Write(peerAddresses->at(i).gameId);
				out->Write((int)peerAddresses->at(i).peers.size());
			}
			sock->Send(out, status);
			delete out;
		}
		//connect
		else if (menuOption == (int)Commands::JOIN_GAME)
		{
			if (status == Status::DONE)
			{
				OutputMemoryStream* out;
				bool aborted = false;
				bool validIdx = false;
				int serverIndex;
				do {
					in = sock->Receive(status);
					mtx.lock();
					in->Read(&serverIndex);
					delete in;
					if (status != Status::DONE || serverIndex == -1) {
						aborted = true;
						mtx.unlock();
						continue;
					}
					validIdx = false;
					for (auto it = peerAddresses->begin(); it != peerAddresses->end(); it++)
					{
						if (it->gameId == serverIndex)
							validIdx = true;
					}

					out = new OutputMemoryStream();
					out->Write(validIdx);
					sock->Send(out, status);
					delete out;
					mtx.unlock();
				} while (!validIdx);


				//std::string msg = "";
				out = new OutputMemoryStream();
				if (peerAddresses->at(serverIndex).pwd != "")
					out->WriteString("Server protected with password");
				else
					out->WriteString("");

				sock->Send(out, status);
				delete out;

				bool validPassword = peerAddresses->at(serverIndex).pwd == "";
				while (!validPassword)
				{
					in = sock->Receive(status);

					if (status == Status::DONE)
					{
						mtx.lock();
						std::string msg = in->ReadString();
						delete in;
						mtx.unlock();
						if (status != Status::DONE || msg == "exit")
						{
							aborted = true;
							continue;
						}
						/*if (msg != peerAddresses->at(serverIndex).pwd)
						{
							int msg2 = 0;
							out->Write(msg2);
							sock->Send(out, status);
						}*/
						validPassword = msg == peerAddresses->at(serverIndex).pwd;
						out = new OutputMemoryStream();
						out->Write(validPassword);
						sock->Send(out, status);
						delete out;

						/*if (!validPassword)
						{
							out = new OutputMemoryStream();
							msg = "Write the password. Write exit to leave";
							out->WriteString(msg);
							sock->Send(out, status);
							delete out;
						}*/

					}
				}


				std::cout << "connect! " << peerAddresses->size() - 1 << " " << serverIndex << std::endl;

				if (peerAddresses->size() - 1 >= serverIndex)
					ConnectToServer(peerAddresses, sock, serverIndex);
				break;
			}
		}
		else
		{
			OutputMemoryStream* out = new OutputMemoryStream();
			out->WriteString("");

			sock->Send(out, status);
			delete in;
		}
	};
}

void ServerCountDown(std::vector<Game>* _peerAddresses)
{
	while (true)
	{
		clock_t currClock = clock();
		for (size_t i = 0; i < peerTimers.size(); i++)
		{
			if (peerTimers[i] < currClock)
			{
				mtx.lock();
				peerTimers.erase(peerTimers.begin() + i);
				_peerAddresses->erase(_peerAddresses->begin() + i);
				i--;
				mtx.unlock();
			}
		}
	}
}

void AcceptConnections(std::vector<Game>* peerAddresses) {
	TcpListener listener;
	Status status = listener.Listen(50000);
	if (status != Status::DONE) {
		return;
	}

	while (true) {
		//Accept
		TcpSocket* sock = new TcpSocket();
		status = listener.Accept(*sock);
		if (status != Status::DONE) {
			delete sock;
			continue;
		}

		std::thread clientMenu(ClientMenu, sock, peerAddresses);

		clientMenu.detach();
	}

	listener.Close();

}


int main() {
	std::vector<Game> peerAddresses;

	//std::thread tServerTimedown(ServerCountDown, &peerAddresses);
	//tServerTimedown.detach();

	AcceptConnections(&peerAddresses);



	return 0;
}