
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
			PeerAddress current = peerAddresses->at(i);
			std::cout << peerAddresses->at(i).ip << ", " << peerAddresses->at(i).port << std::endl;
			out->WriteString(current.ip);
			out->Write(current.port);
		}
		sock->Send(out, status);
		delete out;
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