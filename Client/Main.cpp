
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
bool end = false;

sf::Mutex mtx;

void SendMessages(std::vector<sf::TcpSocket*>* _socks) {
	while (!end) {
		std::string msg;
		std::cin >> msg;

		sf::Packet pack;
		pack << msg;
		mtx.lock();
		for (int i = 0; i < _socks->size(); i++) {
			_socks->at(i)->send(pack);
		}
		if (msg == "e") {
			end = true;
			for (int i = 0; i < _socks->size(); i++) {
				_socks->at(i)->disconnect();
			}
		}
		mtx.unlock();

	}

}

void ReceiveMessages(std::vector<sf::TcpSocket*>* _socks, sf::TcpSocket* _sock) {
	while (!end) {
		sf::Packet pack;
		sf::Socket::Status status = _sock->receive(pack);

		mtx.lock();
		std::string msg;
		pack >> msg;
		if (msg == "e" || status != sf::Socket::Done) {
			std::cout << "Socket with ip: " << _sock->getRemoteAddress() << " and port: " << _sock->getLocalPort() << " was disconnected" << std::endl;
			for (auto it = _socks->begin(); it != _socks->end(); it++) {
				if (*it == _sock) {
					_socks->erase(it);
					delete _sock;
					mtx.unlock();

					return;
				}
			
			}
		}

		std::cout << msg << std::endl;
		mtx.unlock();
	}

}


void AcceptPeers(std::vector<sf::TcpSocket*>* _socks) {
	sf::TcpListener listener;
	listener.listen(localPort);

	while (_socks->size() < 3 && !end)
	{
		sf::TcpSocket* sock = new sf::TcpSocket();
		sf::Socket::Status status = listener.accept(*sock);
		if (status == sf::Socket::Status::Done) {
			_socks->push_back(sock);
			std::cout << "Connected with ip: " << sock->getRemoteAddress() << " and port: " << sock->getLocalPort() << std::endl;

			std::thread tReceive(ReceiveMessages, _socks, _socks->at(_socks->size() - 1));
			tReceive.detach();
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
	sf::Uint64 socketNum;
	pack >> socketNum;
	std::cout << socketNum << std::endl;
	serverSock.disconnect();
	for (int i = 0; i < socketNum; i++) {
		PeerAddress address;
		sf::TcpSocket *sock = new sf::TcpSocket();
		pack >> address.ip >> address.port;
		status = sock->connect(address.ip, address.port);
		if (status == sf::Socket::Status::Done) {
			_socks->push_back(sock);
			std::cout << "Connected with ip: " << sock->getRemoteAddress() << " and port: " << sock->getLocalPort() << std::endl;
		}
	}

	std::thread tAccept(AcceptPeers, _socks);
	tAccept.detach();
	std::thread tSend(SendMessages, _socks);
	tSend.detach();
	for (int i = 0; i < _socks->size(); i++) {
		std::thread tReceive(ReceiveMessages, _socks, _socks->at(i));
		tReceive.detach();
	}

}


int main() {
	std::vector<sf::TcpSocket*> socks;
	ConnectPeer2Peer(&socks);

	while(!end){}

	return 0;
}