
#include <iostream>
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

		switch (status)
		{
		case sf::Socket::Done:
			pack >> str;
			pack.clear();

			switch (state)
			{
			case NULL:
				if (str == "H") {

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

				break;

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