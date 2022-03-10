
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <mutex>
#include "..\res\TcpSocket.h"
#include "..\res\TcpListener.h"

class OutputMemoryStream;
class InputMemoryStream;
class TcpSocket;
class TcpListener;

//std::mutex mtxConexiones;

//struct PeerAddress {
//	std::string ip;
//	unsigned short port;
//};

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

	PeerAddress address;
	address.ip = sock->GetRemoteAddress();
	address.port = sock->GetRemotePort();
	peerAddresses->at(serverIndex).push_back(address);

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
		int menuOption;
		in->Read(&menuOption);

			//create game
		if(menuOption == 1) 
		{
			std::vector<PeerAddress> newConns;
			peerAddresses->push_back(newConns);
			ConnectToServer(peerAddresses, sock, peerAddresses->size()-1);
			break;
		}
			//search game
		if(menuOption == 2) 
		{
			OutputMemoryStream* out = new OutputMemoryStream();
			for (size_t i = 0; i < peerAddresses->size(); i++)
			{
				if (peerAddresses->at(i).size() > 0)
				{
					PeerAddress current = peerAddresses->at(i).at(0);
					out->WriteString(current.ip);
					out->Write(current.port);
				}
			}
			sock->Send(out, status);
		}
			//connect
		if(menuOption == 3) 
		{
			in = sock->Receive(status);
			int serverIndex;
			in->Read(&serverIndex);
			ConnectToServer(peerAddresses, sock, serverIndex);
			break;
		}
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
	AcceptConnections(&peerAddresses);

	/*std::string msg = "localhost";
	std::vector<char> str (msg.begin(), msg.end());
	OutputMemoryStream oms;
	oms.WriteString(msg);


	str.clear();
	InputMemoryStream ims (oms.GetBufferPtr(), oms.GetLength());
	msg = ims.ReadString();
	std::cout << std::string(str.begin(), str.end()) << std::endl;
	system("pause");*/

	return 0;
}