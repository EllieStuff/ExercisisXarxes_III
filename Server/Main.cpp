
#include <iostream>
#include <SFML/Network.hpp>
#include <vector>
#include <thread>
#include <chrono>


void AceptarConexiones(std::vector<sf::TcpSocket*>* _clientes, bool* _end) {
	sf::TcpListener listener;
	sf::Socket::Status status = listener.listen(50000);
	if (status != sf::Socket::Status::Done) {
		return;
	}


	while (!(*_end)) {
		//Accept
		sf::TcpSocket* sock = new sf::TcpSocket();
		status = listener.accept(*sock);
		if (status != sf::Socket::Status::Done) {
			delete sock;
			continue;
		}

		//Guardar en clients
		_clientes->push_back(sock);

	}

}

void EnvioPeriodico(std::vector<sf::TcpSocket*>* _clientes, bool* _end) {
	sf::Packet pack;
	std::string str = "Mensaje desde el servidor\n";
	pack << str;

	while (!(*_end)) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
		for (size_t i = 0; i < _clientes->size(); i++) {
			sf::Socket::Status status = _clientes->at(i)->send(pack);
			switch (status)
			{
			case sf::Socket::Done:
				continue;


			case sf::Socket::Disconnected:
				_clientes->at(i)->disconnect();
				delete _clientes->at(i);
				_clientes->erase(_clientes->begin() + i);
				i--;

				break;


			default:
				break;

			}

		}

	}
}


int main() {
	std::vector<sf::TcpSocket*> clientes;
	char opc;
	bool end = false;
	std::thread tAccepts(AceptarConexiones, &clientes, &end);
	tAccepts.detach();
	std::thread tSends(EnvioPeriodico, &clientes, &end);
	tSends.detach();

	do {
		std::cin >> opc;
	} while (opc != 'e');

	end = true;


	return 0;
}