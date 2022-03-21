#pragma once
#include "Player.h"
#include <mutex>

class TcpSocket;

class GameManager
{
	unsigned int localPort = 0;

	std::vector<TcpSocket*>* socks = new std::vector<TcpSocket*>();
	Player* player = new Player();
	Deck* deck = new Deck();
	Table* table = new Table();
	int* currentTurn = new int(0);
	bool* endRound = new bool(false);
	std::mutex mtx;

	bool ready = false;
	int* playersReady = new int(0);

	std::vector<Pair_Organ_Player> playerTurnOrder;

	void UpdateTurn();
	void ReceiveMessages(TcpSocket* _sock, int* _sceneState);
	void AcceptConnections(int* _sceneState);
	void SendReady();

public:
	GameManager() {}
	~GameManager();

	void CalculateOrganQuantity();
	void ConnectP2P(TcpSocket* _serverSock, int* _sceneState);

	bool Update();
	void Start();

	void SetReady();

	void CreateGame(TcpSocket* serverSock);
	void ListCurrentGames(TcpSocket* serverSock);
	void JoinGame(TcpSocket* serverSock);

	void SetPort(unsigned int _port) { localPort = _port; };
	void SetEndRound(bool _round) { *endRound = _round; }
	int GetPlayersNum() { return socks->size() + 1; };

	bool GetReady() { return ready; }
	int GetPlayersReady() { return *playersReady; }
};