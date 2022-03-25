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
	int gameMaxSize = 3;

	bool ready = false;
	int* playersReady = new int(0);

	std::vector<Pair_Organ_Player> playerTurnOrder;

	void UpdateTurn(bool plus);
	void ReceiveMessages(TcpSocket* _sock, int* _sceneState);
	void AcceptConnections(int* _sceneState);
	void SendReady();

public:
	GameManager() {}
	~GameManager();

	void CheckArray();

	void CalculateOrganQuantity();
	void ConnectP2P(TcpSocket* _serverSock, int* _sceneState);

	bool Update();
	void Start();

	void SetReady();

	void CreateGame(TcpSocket* _serverSock);
	void ListCurrentGames(TcpSocket* _serverSock);
	void JoinGame(TcpSocket* _serverSock, bool& _aborted);

	void SetPort(unsigned int _port) { localPort = _port; };
	void SetEndRound(bool _round) { *endRound = _round; }
	int GetPlayersNum() { return socks->size() + 1; };
	int GetGameSize() { return gameMaxSize; }

	bool GetReady() { return ready; }
	int GetPlayersReady() { return *playersReady; }
};