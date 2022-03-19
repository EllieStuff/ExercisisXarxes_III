#pragma once
#include "Player.h"

class TcpSocket;

class GameManager
{
	unsigned int localPort;

	std::vector<TcpSocket*>* socks = new std::vector<TcpSocket*>();
	Player* player = new Player();
	Deck* deck = new Deck();
	Table* table = new Table();

	void CalculateTurn();

public:
	GameManager();
	~GameManager();

	void ConnectP2P(TcpSocket& _serverSock, int* _sceneState);
	void SendMessages(int* _sceneState);
	void ReceiveMessages(TcpSocket* _sock, int* _sceneState);
	void AcceptConnections(int* _sceneState);

	void SetPort(unsigned int _port) { localPort = _port; };

	void CreateGame(TcpSocket& serverSock);
	void ListCurrentGames(TcpSocket& serverSock);
	void JoinGame(TcpSocket& serverSock);
};