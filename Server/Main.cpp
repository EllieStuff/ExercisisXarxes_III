
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


void AcceptConnections(std::vector<PeerAddress>* peerAddresses) {
	TcpListener listener;
	Status status = listener.Listen(50000);
	if (status != Status::DONE) {
		return;
	}


	while (peerAddresses->size() < 4) {
		//Accept
		TcpSocket* sock = new TcpSocket();
		status = listener.Accept(*sock);
		if (status != Status::DONE) {
			delete sock;
			continue;
		}

		//mtxConexiones.lock();
		std::cout << "Connected with " << sock->GetRemoteAddress() << ". Curr Size = " << peerAddresses->size() << std::endl;
		OutputMemoryStream* out = new OutputMemoryStream();
		out->Write(peerAddresses->size());
		//Packet pack;
		//pack << (Uint64)peerAddresses.size();
		for (int i = 0; i < peerAddresses->size(); i++) {
			//pack << peerAddresses[i].ip << peerAddresses[i].port;
			std::cout << peerAddresses->at(i).ip << ", " << peerAddresses->at(i).port << std::endl;
			out->WriteString(peerAddresses->at(i).ip);
			out->Write(peerAddresses->at(i).port);
		}
		sock->Send(out, status);
		std::cout << (int)status << std::endl;

		PeerAddress address;
		address.ip = sock->GetRemoteAddress();
		address.port = sock->GetRemotePort();
		peerAddresses->push_back(address);

		//mtxConexiones.unlock();

		sock->Disconnect();

	}

	listener.Close();

}


int main() {
	std::vector<PeerAddress> peerAddresses;
	AcceptConnections(&peerAddresses);


	return 0;
}