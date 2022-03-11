
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

		OutputMemoryStream* out = new OutputMemoryStream();
		out->WriteString(msg);
		//mtx.lock();
		std::cout << _socks->size() << std::endl;
		for (int i = 0; i < _socks->size(); i++) {
			Status status;
			_socks->at(i)->Send(out, status);
			std::cout << (int)status << std::endl;
		}
		if (msg == "e") {
			end = true;
			for (int i = 0; i < _socks->size(); i++) {
				_socks->at(i)->Disconnect();
				delete _socks->at(i);
			}
			_socks->clear();
		}
		//mtx.unlock();
		delete out;
	}
	
}

void ReceiveMessages(std::vector<TcpSocket*>* _socks, TcpSocket* _sock) {
	while (!end) {
		//Packet pack;
		//Socket::Status status = _sock->receive(pack);
		Status status;
		InputMemoryStream* in = _sock->Receive(status);
		if (status == Status::DISCONNECTED)
		{
			delete in;
			return;
		}
		//mtx.lock();

		std::string msg = in->ReadString();
		std::cout << msg << std::endl;
		if (msg == "e" || status != Status::DONE) {
			std::cout << "Socket with ip: " << _sock->GetRemoteAddress() << " and port: " << _sock->GetLocalPort() << " was disconnected" << std::endl;
			for (auto it = _socks->begin(); it != _socks->end(); it++) {
				if (*it == _sock) {
					_socks->erase(it);
					delete _sock;
					delete in;
					return;
				}
			}
		}

		delete in;
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

void ConnectPeer2Peer(std::vector<TcpSocket*>* _socks) 
{
	TcpSocket serverSock;
	Status status = serverSock.Connect("127.0.0.1", 50000);
	if (status != Status::DONE) {
		return;
	}
	localPort = serverSock.GetLocalPort();

	std::cout << "Welcome!" << std::endl;
	std::cout << "1. Create P2P Game" << std::endl;
	std::cout << "2. List P2P Games" << std::endl;
	std::cout << "3. Join P2P Game" << std::endl;

	while (true)
	{
		std::cout << "\nSelect option: ";
		int option;
		std::cin >> option;
		if (option <= 0 || option > 3)
			continue;

		OutputMemoryStream* out = new OutputMemoryStream();
		out->Write(option);

		serverSock.Send(out, status);

		if (option == 3)
		{
			std::cout << "Type server number" << std::endl;
			int server;
			std::cin >> server;

			delete out;
			out = new OutputMemoryStream();

			out->Write(server);
			serverSock.Send(out, status);

			delete out;
		}
		else if (option == 2)
		{
			InputMemoryStream* in = serverSock.Receive(status);
			int size;
			in->Read(&size);
			for (int i = 0; i < size; i++)
			{
				PeerAddress peerAddress;
				peerAddress.ip = in->ReadString();
				in->Read(&peerAddress.port);
				std::cout << "Game idx: " << i << ", Game ip: " << peerAddress.ip << ", Game Port: " << peerAddress.port << std::endl;
			}

			delete in;
		}

		if (option == 1 || option == 3)
		{
			InputMemoryStream* in = serverSock.Receive(status);
			int socketNum;
			in->Read(&socketNum);

			serverSock.Disconnect();

			bool started = false;
			for (int i = 0; i < socketNum; i++)
			{
				PeerAddress address;
				TcpSocket* sock = new TcpSocket();
				std::string msg;

				int num;

				if (!started)
				{
					in->Read(&num);
					std::cout << num << std::endl;
					started = true;
				}

				msg = in->ReadString();
				in->Read(&address.port);
				std::cout << "Connected with ip: " << msg << " and port: " << address.port << std::endl;
				status = sock->Connect(msg, address.port);

				if (status == Status::DONE)
				{
					_socks->push_back(sock);
					std::cout << "Connected with ip: " << sock->GetRemoteAddress() << " and port: " << sock->GetLocalPort() << std::endl;
				}
			}
			delete in;

			std::thread tAccept(AcceptPeers, _socks);
			tAccept.detach();
			std::thread tSend(SendMessages, _socks);
			tSend.detach();
			for (int i = 0; i < _socks->size(); i++)
			{
				std::thread tReceive(ReceiveMessages, _socks, _socks->at(i));
				tReceive.detach();
			}
			break;
		}
	}
}


int main() {
	std::vector<TcpSocket*> socks;
	ConnectPeer2Peer(&socks);

	while(!end){}

	return 0;
}