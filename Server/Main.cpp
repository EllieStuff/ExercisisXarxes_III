
#include <iostream>
#include <SFML/Network.hpp>
#include <vector>
#include <thread>
#include <chrono>
#include <mutex>


std::mutex mtxConexiones;

struct PeerAddress {
	std::string ip;
	unsigned short port;
};
std::vector<PeerAddress> peerAddresses;


void AcceptConnections() {
	sf::TcpListener listener;
	sf::Socket::Status status = listener.listen(50000);
	if (status != sf::Socket::Status::Done) {
		return;
	}


	while (peerAddresses.size() < 4) {
		//Accept
		sf::TcpSocket* sock = new sf::TcpSocket();
		status = listener.accept(*sock);
		if (status != sf::Socket::Status::Done) {
			delete sock;
			continue;
		}

		mtxConexiones.lock();
		sf::Packet pack;
		pack << peerAddresses.size();
		for (int i = 0; i < peerAddresses.size(); i++) {
			pack << peerAddresses[i].ip << peerAddresses[i].port;
		}
		sock->send(pack);

		PeerAddress address;
		address.ip = sock->getRemoteAddress().toString();
		address.port = sock->getRemotePort();
		peerAddresses.push_back(address);

		mtxConexiones.unlock();

		sock->disconnect();

	}

	listener.close();

}


int main() {

	AcceptConnections();


	return 0;
}