
#include <iostream>
#include <SFML/Network.hpp>
//#include <vector>
//#include <thread>
//#include <chrono>

// Mirar ip en consola amb ipconfig, sino, es pot fer en mateix PC amb "127.0.0.1" o "localHost"

int main() {
	sf::TcpSocket sock;
	sf::Socket::Status status = sock.connect("127.0.0.1", 50000);
	bool end = false;

	do {
		sf::Packet pack;
		status = sock.receive(pack);
		std::string str;

		switch (status)
		{
		case sf::Socket::Done:
			pack >> str;
			std::cout << str;

			break;


		case sf::Socket::Disconnected:
			end = true;

			break;


		default:
			break;
		}

	} while (!end);


	return 0;
}