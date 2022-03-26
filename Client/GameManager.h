#pragma once
#include "Player.h"
#include <mutex>
#include <list>
#include "../res/InputMemoryStream.h"
#include "../res/TcpSocket.h"
#include "../res/TcpListener.h"

class TcpSocket;

class GameManager
{
	TcpListener listener;
	unsigned int localPort = 0;

	std::list<TcpSocket*>* socks = new std::list<TcpSocket*>();
	Player* player = new Player();
	Deck* deck = new Deck();
	Table* table = new Table();
	int* currentTurn = new int(0);
	bool* endRound = new bool(false);
	std::mutex mtx;

	bool ready = false;
	int* playersReady = new int(0);

	std::vector<Pair_Organ_Player> playerTurnOrder;

	void UpdateTurn(bool plus);
	void ReceiveMessages(InputMemoryStream in1);
	void AcceptConnections(int* _sceneState);
	void SendReady();

public:
	GameManager() {}
	~GameManager();

	void ListEnemiesWithTheirCards();

	bool Threatment();

	bool PlaceInfection();

	bool VaccineOrgan();

	void CheckArray();

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