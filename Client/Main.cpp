
#include <iostream>
#include <vector>
#include <thread>
#include "..\res\TcpSocket.h"
#include "..\res\TcpListener.h"

// Mirar ip en consola amb ipconfig, sino, es pot fer en mateix PC amb "127.0.0.1" o "localHost"

unsigned short localPort;
bool end = false;

//sf::Mutex mtx;

void SendMessages(std::vector<TcpSocket*>* _socks) {
	while (!end) {
		std::string msg;
		std::cin >> msg;

		//Packet pack;
		//pack << msg;
		OutputMemoryStream out;
		out.WriteString(msg);
		//mtx.lock();
		for (int i = 0; i < _socks->size(); i++) {
			Status status;
			_socks->at(i)->Send(&out, status);
		}
		if (msg == "e") {
			end = true;
			for (int i = 0; i < _socks->size(); i++) {
				_socks->at(i)->Disconnect();
			}
		}
		//mtx.unlock();

	}

}

void ReceiveMessages(std::vector<TcpSocket*>* _socks, TcpSocket* _sock) {
	while (!end) {
		//Packet pack;
		//Socket::Status status = _sock->receive(pack);
		Status status;
		InputMemoryStream in = *_sock->Receive(status);

		//mtx.lock();
		std::string msg = in.ReadString();
		//pack >> msg;
		if (msg == "e" || status != Status::DONE) {
			std::cout << "Socket with ip: " << _sock->GetRemoteAddress() << " and port: " << _sock->GetLocalPort() << " was disconnected" << std::endl;
			for (auto it = _socks->begin(); it != _socks->end(); it++) {
				if (*it == _sock) {
					_socks->erase(it);
					delete _sock;
					//mtx.unlock();

					return;
				}
			
			}
		}

		std::cout << msg << std::endl;
		//mtx.unlock();
	}

}


void AcceptPeers(std::vector<TcpSocket*>* _socks) {
	TcpListener listener;
	listener.Listen(localPort);

	while (_socks->size() < 3 && !end)
	{
		TcpSocket* sock = new TcpSocket();
		Status status = listener.Accept(*sock);
		if (status == Status::DONE) {
			_socks->push_back(sock);
			std::cout << "Connected with ip: " << sock->GetRemoteAddress() << " and port: " << sock->GetLocalPort() << std::endl;

			std::thread tReceive(ReceiveMessages, _socks, _socks->at(_socks->size() - 1));
			tReceive.detach();
		}
	}

	listener.Close();

}

void ConnectPeer2Peer(std::vector<TcpSocket*>* _socks) {
	TcpSocket serverSock;
	Status status = serverSock.Connect("127.0.0.1", 50000);
	if (status != Status::DONE) {
		return;
	}
	localPort = serverSock.GetLocalPort();

	//Packet pack;
	InputMemoryStream* in = serverSock.Receive(status);
	int socketNum;
	in->Read(&socketNum);
	//sf::Uint64 socketNum;
	//pack >> socketNum;
	std::cout << socketNum << std::endl;
	serverSock.Disconnect();
	for (int i = 0; i < socketNum; i++) {
		PeerAddress address;
		TcpSocket *sock = new TcpSocket();
		//pack >> address.ip >> address.port;
		address.ip = in->ReadString();
		in->Read(&address.port);
		status = sock->Connect(address.ip, address.port);
		if (status == Status::DONE) {
			_socks->push_back(sock);
			std::cout << "Connected with ip: " << sock->GetRemoteAddress() << " and port: " << sock->GetLocalPort() << std::endl;
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
	std::vector<TcpSocket*> socks;
	ConnectPeer2Peer(&socks);

	while(!end){}

	return 0;
}