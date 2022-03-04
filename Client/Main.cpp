
#include <iostream>
<<<<<<< Updated upstream
#include <SFML/Network.hpp>
//#include <vector>
#include <thread>
//#include <chrono>

// Mirar ip en consola amb ipconfig, sino, es pot fer en mateix PC amb "127.0.0.1" o "localHost"

char state = NULL;


void Recepcion(sf::TcpSocket* _sock, bool* _end) {
	sf::Socket::Status status;
	do {
		sf::Packet pack;
		status = _sock->receive(pack);
		std::string str;
=======
#include <vector>
#include <thread>
#include "InputMemoryStream.h"
#include "OutputMemoryStream.h"

// Mirar ip en consola amb ipconfig, sino, es pot fer en mateix PC amb "127.0.0.1" o "localHost"

unsigned short localPort;
bool end = false;

sf::Mutex mtx;
InputMemoryStream input;
OutputMemoryStream output;

void SendMessages(std::vector<sf::TcpSocket*>* _socks) {
	while (!end) 
	{
		std::string msg;
		std::cin >> msg;

		output.SerializeString(msg);

		mtx.lock();
		output.SendInfo(_socks);
		if (msg == "e") {
			end = true;
			for (int i = 0; i < _socks->size(); i++) {
				_socks->at(i)->disconnect();
				std::cout << "test2" << std::endl;
			}
		}
		mtx.unlock();
>>>>>>> Stashed changes

		switch (status)
		{
		case sf::Socket::Done:
			pack >> str;
			pack.clear();

			switch (state)
			{
			case NULL:
				if (str == "H") {

<<<<<<< Updated upstream
				}
				else if (str == "J") {

				}
				else {
					std::cout << str;
					str = "";
					std::cin >> str;
					pack << str;
					_sock->send(pack);
				}
=======
void ReceiveMessages(std::vector<sf::TcpSocket*>* _socks, sf::TcpSocket* _sock) {
	while (!end) {
		std::string msg = input.ReceiveInfo(_sock);
		mtx.lock();
		if (msg == "e" && input.disconnectClient(_socks, _sock))
		{
			std::cout << "Socket with ip: " << _sock->getRemoteAddress() << " and port: " << _sock->getLocalPort() << " was disconnected" << std::endl;
		}
		std::cout << msg << std::endl;
		mtx.unlock();
	}
>>>>>>> Stashed changes

				break;

<<<<<<< Updated upstream
			case 'H':

				break;

			case 'J':

				break;

			default:
				break;
			}

			break;


		case sf::Socket::Disconnected:
			*_end = true;

			break;


		default:
			break;
		}

	} while (!*_end);
=======
void AcceptPeers(std::vector<sf::TcpSocket*>* _socks) {
	
	input.StartListener(localPort);
	
	while (_socks->size() < 3 && !end)
	{
		if (input.ReceiveConnection(_socks)) 
		{
			std::thread tReceive(ReceiveMessages, _socks, _socks->at(_socks->size() - 1));
			tReceive.detach();
		}
	}

	input.Close();
}

void ConnectPeer2Peer(std::vector<sf::TcpSocket*>* _socks) {

	if(output.ConnectP2P()) 
	{
		std::thread tAccept(AcceptPeers, _socks);
		tAccept.detach();
		std::thread tSend(SendMessages, _socks);
		tSend.detach();
		for (int i = 0; i < _socks->size(); i++)
		{
			std::cout << "test" << std::endl;
			std::thread tReceive(ReceiveMessages, _socks, _socks->at(i));
			tReceive.detach();
		}
	}
>>>>>>> Stashed changes

}


int main() {
	sf::TcpSocket sock;
	sf::Socket::Status status = sock.connect("127.0.0.1", 50000);
	bool end = false;

	std::thread tReceive(Recepcion, &sock, &end);
	tReceive.detach();

	std::string str;
	do {
		std::cin >> str;
		sf::Packet pack;
		pack << str;
		sock.send(pack);

	} while (str != "e");

	end = true;

	sock.disconnect();

	return 0;
}