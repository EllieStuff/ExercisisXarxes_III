#pragma once
#include "Deck.h"

class GameManager
{
	unsigned int localPort;
	

	Deck deck;
	std::vector<TcpSocket*> socks;

	void CalculateTurn();

public:
	GameManager();
	~GameManager();

	void ConnectP2P();
	void SendMessages();
	void ReceiveMessages(int _sockIdx);
	void AcceptConnections(int* sceneState);

	void SetPort(unsigned int _port) { localPort = _port };
};