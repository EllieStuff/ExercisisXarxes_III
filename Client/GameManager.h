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
	int* currentTurn = new int(0);
	bool* endRound;

	std::vector<Pair_Organ_Player> playerTurnOrder;

	void UpdateTurn();

public:
	GameManager();
	~GameManager();

	void CalculateOrganQuantity();
	void ConnectP2P(TcpSocket& _serverSock, int* _sceneState);

	bool Update();
	void Start();

	void ReceiveMessages(TcpSocket* _sock, int* _sceneState);
	void AcceptConnections(int* _sceneState);

	void SetPort(unsigned int _port) { localPort = _port; };

	void CreateGame(TcpSocket& serverSock);
	void ListCurrentGames(TcpSocket& serverSock);
	void JoinGame(TcpSocket& serverSock);

	void SetEndRound(bool _round) { *endRound = _round; }
};