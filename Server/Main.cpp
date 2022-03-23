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
	OutputMemoryStream* out = new OutputMemoryStream();
	out->Write(peerAddresses->at(serverIndex).peers.size());
	for (int i = 0; i < peerAddresses->at(serverIndex).peers.size(); i++) {
		PeerAddress current = peerAddresses->at(serverIndex).peers[i];
		std::cout << peerAddresses->at(serverIndex).peers[i].ip << ", " << peerAddresses->at(serverIndex).peers[i].port << std::endl;
		out->WriteString(current.ip);
		out->Write(current.port);
	}
	sock->Send(out, status);
	delete out;
	std::cout << (int)status << std::endl;

	if (peerAddresses->at(serverIndex).peers.size() < 3)
	{
		PeerAddress address;
		address.ip = sock->GetRemoteAddress();
		address.port = sock->GetRemotePort();
		peerAddresses->at(serverIndex).peers.push_back(address);

		peerTimers[serverIndex] = clock() + closeTime;
	}
	else
	{
		peerAddresses->erase(peerAddresses->begin() + serverIndex);
		peerTimers.erase(peerTimers.begin() + serverIndex);
	}

	sock->Disconnect();
}

void ClientMenu(TcpSocket* sock, std::vector<Game>* peerAddresses)
{
	std::cout << "1" << std::endl;
	Status status;
	std::cout << "2" << std::endl;
	std::cout << "Connected with " << sock->GetRemoteAddress() << std::endl;
	std::cout << "3" << std::endl;
	//sending menu int;

	while (true)
	{
		std::cout << "4" << std::endl;
		InputMemoryStream* in = sock->Receive(status);
		std::cout << "5" << std::endl;
		if (status != Status::DONE)
			continue;
		std::cout << "6" << std::endl;

		int menuOption;

		std::cout << "7" << std::endl;

		if (in == nullptr)
			break;

		std::cout << "8" << std::endl;

		in->Read(&menuOption);

		std::cout << "9" << std::endl;

		//create game
		if (menuOption == (int)Commands::CREATE_GAME)
		{
			//mtx.lock();
			std::cout << "10" << std::endl;
			std::cout << "11" << std::endl;
			peerAddresses->push_back(Game());
			std::cout << "12" << std::endl;
			int size = peerAddresses->size() - 1;
			std::cout << size << std::endl;
			std::cout << peerAddresses->size() << std::endl;
			std::cout << "13" << std::endl;
			//mtx.unlock();
			std::cout << "14" << std::endl;

			std::string msg = "Write the password if you want (write '-' if you don't)";
			std::cout << "15" << std::endl;
			OutputMemoryStream out;
			std::cout << "16" << std::endl;
			out.WriteString(msg);
			std::cout << "17" << std::endl;
			sock->Send(&out, status);
			std::cout << "18" << std::endl;

			delete in;
			std::cout << "19" << std::endl;
			Status status;
			std::cout << "20" << std::endl;
			InputMemoryStream* in = sock->Receive(status);
			std::cout << "21" << std::endl;

			msg = in->ReadString();

			std::cout << "22" << std::endl;

			if (msg._Equal("-")) msg = "";

			std::cout << "23" << std::endl;

			peerTimers.push_back(clock() + closeTime);

			std::cout << "24" << std::endl;
			std::cout << size << std::endl;
			std::cout << peerAddresses->size() << std::endl;
			//PETA AQUI
			peerAddresses->at(size).gameId = currGameId;

			std::cout << "25" << std::endl;

			peerAddresses->at(size).pwd = msg;

			std::cout << "26" << std::endl;

			ConnectToServer(peerAddresses, sock, size);

			std::cout << "27" << std::endl;

			delete in;

			std::cout << "28" << std::endl;

			currGameId++;

			std::cout << "29" << std::endl;
			break;
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
			std::cout << "30" << std::endl;
			delete in;
			std::cout << "31" << std::endl;
			std::cout << "32" << std::endl;
			InputMemoryStream* in = sock->Receive(status);
			std::cout << "33" << std::endl;
			int serverIndex;
			std::cout << "34" << std::endl;
			while (in == nullptr)
				in = sock->Receive(status);
			std::cout << "35" << std::endl;
			in->Read(&serverIndex);
			std::cout << "36" << std::endl;
			std::string msg = "";
			std::cout << "37" << std::endl;
			OutputMemoryStream* out = new OutputMemoryStream();
			std::cout << "38" << std::endl;
			if (peerAddresses->at(serverIndex).pwd != "")
				out->WriteString("Server protected with password");
			else
				out->WriteString("");

			std::cout << "39" << std::endl;

			sock->Send(out, status);

			std::cout << "40" << std::endl;

			while (peerAddresses->at(serverIndex).pwd != msg)
			{
				std::cout << "41" << std::endl;
				delete out;
				delete in;
				std::cout << "42" << std::endl;
				out = new OutputMemoryStream();
				std::cout << "43" << std::endl;
				msg = "Write the password. Write exit to leave";
				std::cout << "44" << std::endl;
				std::cout << "Type password" << std::endl;
				std::cout << "45" << std::endl;
				out->WriteString(msg);
				std::cout << "46" << std::endl;
				sock->Send(out, status);
				std::cout << "47" << std::endl;
				in = sock->Receive(status);
				std::cout << "48" << std::endl;
				if (in == nullptr)
					break;
				std::cout << "49" << std::endl;
				msg = in->ReadString();
				std::cout << "50" << std::endl;
				if (status != Status::DONE || msg == "exit")
				{
					std::cout << "51_E" << std::endl;
					break;
				}
				std::cout << "51" << std::endl;
				if (msg != peerAddresses->at(serverIndex).pwd)
				{
					std::cout << "52" << std::endl;
					std::string msg2 = "Incorrect password. Try again or write 'exit' to leave";
					std::cout << "53" << std::endl;
					out->WriteString(msg2);
					std::cout << "54" << std::endl;
					sock->Send(out, status);
					std::cout << "55" << std::endl;
				}

			}

			delete out;

			std::cout << "connect! " << peerAddresses->size() - 1 << " " << serverIndex << std::endl;

			if (peerAddresses->size() - 1 >= serverIndex)
			ConnectToServer(peerAddresses, sock, serverIndex);
			std::cout << "56" << std::endl;
			delete in;
			break;
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
				peerTimers.erase(peerTimers.begin() + i);
				_peerAddresses->erase(_peerAddresses->begin() + i);
				i--;
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