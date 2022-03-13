
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

void ConnectToServer(std::vector<std::vector<PeerAddress>>* peerAddresses, TcpSocket* sock, int serverIndex)
{
	Status status;
	std::cout << "Connected with " << sock->GetRemoteAddress() << ". Curr Size = " << peerAddresses->at(serverIndex).size() << std::endl;
	OutputMemoryStream* out = new OutputMemoryStream();
	out->Write(peerAddresses->at(serverIndex).size());
	for (int i = 0; i < peerAddresses->at(serverIndex).size(); i++) {
		PeerAddress current = peerAddresses->at(serverIndex).at(i);
		std::cout << peerAddresses->at(serverIndex).at(i).ip << ", " << peerAddresses->at(serverIndex).at(i).port << std::endl;
		out->WriteString(current.ip);
		out->Write(current.port);
	}
	sock->Send(out, status);
	delete out;
	std::cout << (int)status << std::endl;
	
	if(peerAddresses->at(serverIndex).size() < 3)
	{
		PeerAddress address;
		address.ip = sock->GetRemoteAddress();
		address.port = sock->GetRemotePort();
		peerAddresses->at(serverIndex).push_back(address);

		peerTimers[serverIndex] = clock() + closeTime;
	}
	else
	{
		peerAddresses->erase(peerAddresses->begin() + serverIndex);
		peerTimers.erase(peerTimers.begin() + serverIndex);
	}
	
	sock->Disconnect();
}

void ClientMenu(TcpSocket* sock, std::vector<std::vector<PeerAddress>>* peerAddresses)
{
	Status status;
	std::cout << "Connected with " << sock->GetRemoteAddress() << std::endl;
	//sending menu int;

	while(true) 
	{
		InputMemoryStream* in = sock->Receive(status);
		if (status != Status::DONE)
			continue;

		int menuOption;
		in->Read(&menuOption);

			//create game
		if(menuOption == 1) 
		{
			mtx.lock();
			std::vector<PeerAddress> newConns;
			peerAddresses->push_back(newConns);

			peerTimers.push_back(clock() + closeTime);

			ConnectToServer(peerAddresses, sock, peerAddresses->size() - 1);
			mtx.unlock();
			delete in;
			break;
		}
			//search game
		else if(menuOption == 2) 
		{
			OutputMemoryStream* out = new OutputMemoryStream();
			out->Write((int)peerAddresses->size());
			mtx.lock();
			for (size_t i = 0; i < peerAddresses->size(); i++)
			{
				if (peerAddresses->at(i).size() > 0)
				{
					PeerAddress current = peerAddresses->at(i).at(0);
					out->WriteString(current.ip);
					out->Write(current.port);
				}
				else 
				{
					peerAddresses->erase(peerAddresses->begin() + i);
					peerTimers.erase(peerTimers.begin() + i);
				}
			}
			mtx.unlock();
			sock->Send(out, status);
			delete out;
		}
			//connect
		else if(menuOption == 3) 
		{
			mtx.lock();
			in = sock->Receive(status);
			int serverIndex;
			in->Read(&serverIndex);
			if(peerAddresses->size() - 1 <= serverIndex)
			ConnectToServer(peerAddresses, sock, serverIndex);
			mtx.unlock();
			delete in;
			break;
		}
		delete in;
	}

}

void ServerCountDown(std::vector<std::vector<PeerAddress>>* _peerAddresses)
{
	while(true)
	{
		mtx.lock();
		clock_t currClock = clock();
		for (size_t i = 0; i < peerTimers.size(); i++)
		{
			if(peerTimers[i] < currClock)
			{
				peerTimers.erase(peerTimers.begin() + i);
				_peerAddresses->erase(_peerAddresses->begin() + i);
				i--;
			}
		}
		mtx.unlock();
	}
}

void AcceptConnections(std::vector<std::vector<PeerAddress>>* peerAddresses) {
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
	std::vector<std::vector<PeerAddress>> peerAddresses{};
	
	std::thread tServerTimedown(ServerCountDown, &peerAddresses);
	tServerTimedown.detach();

	AcceptConnections(&peerAddresses);

	

	return 0;
}