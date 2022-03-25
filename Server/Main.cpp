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

std::mutex mtx;
int currGameId = 0;

void ConnectToServer(std::vector<Game>* peerAddresses, TcpSocket* sock, int serverIndex)
{
	Status status;
	std::cout << "Connected with " << sock->GetRemoteAddress() << ". Curr Size = " << peerAddresses->at(serverIndex).peers.size() << std::endl;
	OutputMemoryStream out;
	out.Write(peerAddresses->at(serverIndex).peers.size());

	////----------------
	//mtx.lock();
	for (int i = 0; i < peerAddresses->at(serverIndex).peers.size(); i++) 
	{
		PeerAddress current = peerAddresses->at(serverIndex).peers[i];
		std::cout << peerAddresses->at(serverIndex).peers[i].ip << ", " << peerAddresses->at(serverIndex).peers[i].port << std::endl;
		out.WriteString(current.ip);
		out.Write(current.port);
	}
	sock->Send(&out, status);
	std::cout << (int)status << std::endl;

	if (peerAddresses->at(serverIndex).peers.size() < 3)
	{
		PeerAddress address;
		address.ip = sock->GetRemoteAddress();
		address.port = sock->GetRemotePort();
		
		peerAddresses->at(serverIndex).peers.push_back(address);
		
	}
	else
	{
		peerAddresses->erase(peerAddresses->begin() + serverIndex);
	}
	//mtx.unlock();
	////----------------

	sock->Disconnect();
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

		int menuOption;

		in->Read(&menuOption);
		delete in;

		//create game
		if (menuOption == (int)Commands::CREATE_GAME)
		{
			//----------
			mtx.lock();
			peerAddresses->push_back(Game());
			int size = peerAddresses->size() - 1;
			std::cout << peerAddresses->size() << std::endl;
			mtx.unlock();
			//----------

			std::string msg = "";

			in = sock->Receive(status);

			if(status == Status::DONE) 
			{
				msg = in->ReadString();

				if (msg._Equal("-")) msg = "";

				//----------
				mtx.lock();

				std::cout << size << std::endl;
				std::cout << peerAddresses->size() << std::endl;

				peerAddresses->at(size).gameId = currGameId;

				peerAddresses->at(size).pwd = msg;
				//mtx.unlock();
				////----------

				ConnectToServer(peerAddresses, sock, size);
				
				////----------
				//mtx.lock();
				currGameId++;
				mtx.unlock();
				//----------

				break;
			}
		}
		//search game
		else if (menuOption == (int)Commands::GAME_LIST)
		{
			//----------
			mtx.lock();
			OutputMemoryStream* out = new OutputMemoryStream();
			out->Write((int)peerAddresses->size());

			
			for (size_t i = 0; i < peerAddresses->size(); i++)
			{
				out->Write(peerAddresses->at(i).gameId);
				out->Write((int)peerAddresses->at(i).peers.size());
			}
			mtx.unlock();
			//----------
			sock->Send(out, status);
			delete out;
		}
		//connect
		else if (menuOption == (int)Commands::JOIN_GAME)
		{
			if(status == Status::DONE) 
			{
				OutputMemoryStream* out;
				bool validIdx;
				int serverIndex = false;
				do {
					in = sock->Receive(status);
					in->Read(&serverIndex);
					delete in;

					//----------
					mtx.lock();
					validIdx = serverIndex >= 0 && serverIndex < peerAddresses->size();
					mtx.unlock();
					//----------

					out = new OutputMemoryStream();
					out->Write(validIdx);
					sock->Send(out, status);
					delete out;
				} while (!validIdx);

				//std::string msg = "";
				out = new OutputMemoryStream();

				//----------
				mtx.lock();
				if (peerAddresses->at(serverIndex).pwd != "")
					out->WriteString("Server protected with password");
				else
					out->WriteString("");
				mtx.unlock();
				//----------

				sock->Send(out, status);
				delete out;

				//----------
				mtx.lock();
				bool validPassword = peerAddresses->at(serverIndex).pwd == "";
				mtx.unlock();
				//----------
				while (!validPassword)
				{
					in = sock->Receive(status);

					if(status == Status::DONE) 
					{
						std::string msg = in->ReadString();
						delete in;

						if (status != Status::DONE || msg == "exit")
						{
							break;
						}

						//----------
						mtx.lock();
						validPassword = msg == peerAddresses->at(serverIndex).pwd;
						mtx.unlock();
						//----------

						out = new OutputMemoryStream();
						out->Write(validPassword);
						sock->Send(out, status);
						delete out;

					}
				}

				std::cout << "connect! " << peerAddresses->size() - 1 << " " << serverIndex << std::endl;

				if (peerAddresses->size() - 1 >= serverIndex)
				{
					//----------
					mtx.lock();
					ConnectToServer(peerAddresses, sock, serverIndex);
					mtx.unlock();
					//----------
				}
				break;
			}
		}
		else
		{
			/*OutputMemoryStream* out = new OutputMemoryStream();
			out->WriteString("");

			sock->Send(out, status);*/
		}
	};
}

void Update()
{

}

void ServerControl(std::vector<Game>* peerAddresses)
{
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
							continue;

						int menuOption;

						in->Read(&menuOption);
						delete in;

						if (menuOption == (int)Commands::CREATE_GAME)
						{
							peerAddresses->push_back(Game());
							int size = peerAddresses->size() - 1;
							std::cout << peerAddresses->size() << std::endl;

						}
						//search game
						else if (menuOption == (int)Commands::GAME_LIST)
						{

						}
						//connect
						else if (menuOption == (int)Commands::JOIN_GAME)
						{

						}

					}
				}
			}
		}
	}

	listener.Close();
}

int main() {
	std::vector<Game> peerAddresses;

	std::thread tServer(ServerControl, &peerAddresses);
	tServer.detach();

	Update();

	return 0;
}