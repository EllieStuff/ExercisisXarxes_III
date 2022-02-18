
#include <iostream>
#include <SFML/Network.hpp>
#include <vector>


void AceptarConexiones(std::vector<sf::TcpSocket*>* _clientes) {
	sf::TcpListener listener;
	sf::Socket::Status status = listener.listen(50000);
	if (status != sf::Socket::Status::Done) {
		return;
	}


	while (true) {
		listener.accept

		//Accept
		
		//Guardar en clients


	}

}


int main() {
	std::vector<sf::TcpSocket*> clientes;
	char opc;
	bool end = false;
	do {
		std::cin >> opc;
	} while (!end);


	return 0;
}