
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <mutex>
#include "..\Client\TcpSocket.h"
#include "..\Client\TcpListener.h"


//std::mutex mtxConexiones;

//struct PeerAddress {
//	std::string ip;
//	unsigned short port;
//};
std::vector<PeerAddress> peerAddresses;


void AcceptConnections() {
	TcpListener listener;
	Status status = listener.Listen(50000);
	if (status != Status::DONE) {
		return;
	}


	while (peerAddresses.size() < 4) {
		//Accept
		TcpSocket* sock = new TcpSocket();
		status = listener.Accept(*sock);
		if (status != Status::DONE) {
			delete sock;
			continue;
		}

		//mtxConexiones.lock();
		std::cout << "Connected with " << sock->GetRemoteAddress() << ". Curr Size = " << peerAddresses.size() << std::endl;
		OutputMemoryStream out;
		out.Write(peerAddresses.size());
		//Packet pack;
		//pack << (Uint64)peerAddresses.size();
		for (int i = 0; i < peerAddresses.size(); i++) {
			//pack << peerAddresses[i].ip << peerAddresses[i].port;
			out.WriteString(peerAddresses[i].ip);
			out.Write(peerAddresses[i].port);
		}
		sock->Send(&out, status);

		PeerAddress address;
		address.ip = sock->GetRemoteAddress();
		address.port = sock->GetRemotePort();
		peerAddresses.push_back(address);

		//mtxConexiones.unlock();

		sock->Disconnect();

	}

	listener.Close();

}


int main() {

	AcceptConnections();


	return 0;
}