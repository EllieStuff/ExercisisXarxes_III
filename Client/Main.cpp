
#include <iostream>
#include <SFML/Network.hpp>
#include <vector>
#include <thread>

// Mirar ip en consola amb ipconfig, sino, es pot fer en mateix PC amb "127.0.0.1" o "localHost"

struct PeerAddress {
	std::string ip;
	unsigned short port;
};
unsigned short localPort;



void AcceptPeers(std::vector<sf::TcpSocket*>* _socks) {
	sf::TcpListener listener;
	listener.listen(localPort);

	while (_socks->size() < 3)
	{
		sf::TcpSocket *sock = new sf::TcpSocket();
		sf::Socket::Status status = listener.accept(*sock);
		if (status == sf::Socket::Status::Done) {
			_socks->push_back(sock);
		}
	}

	listener.close();

}


void ConnectPeer2Peer(std::vector<sf::TcpSocket*>* _socks) {
	sf::TcpSocket serverSock;
	sf::Socket::Status status = serverSock.connect("127.0.0.1", 50000);
	if (status != sf::Socket::Status::Done) {
		return;
	}
	localPort = serverSock.getLocalPort();

	sf::Packet pack;
	serverSock.receive(pack);
	int socketNum;
	pack >> socketNum;
	serverSock.disconnect();
	for (int i = 0; i < socketNum; i++) {
		PeerAddress address;
		sf::TcpSocket *sock = new sf::TcpSocket();
		pack >> address.ip >> address.port;
		status = sock->connect(address.ip, address.port);
		if (status == sf::Socket::Status::Done) {
			_socks->push_back(sock);
		}
	}


	//AcceptPeers(_socks);
	std::thread tAccept(AcceptPeers, _socks);
	tAccept.detach();

}


int main() {
	std::vector<sf::TcpSocket*> socks;
	//ConnectPeer2Peer(&socks);
	std::thread tConnect(ConnectPeer2Peer, &socks);
	tConnect.detach();

	while(true){}

	return 0;
}