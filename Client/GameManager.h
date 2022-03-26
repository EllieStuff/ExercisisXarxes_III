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

	void UpdateTurn(bool plus);
	void ReceiveMessages(TcpSocket* _sock, int* _sceneState);
	void AcceptConnections(Selector* selector, TcpListener* listener);
	void SendReady();

	void JoinGame(OutputMemoryStream* out);
	void SendPassword();

	void ConnectP2P(Selector* selector, TcpSocket* serverSock, InputMemoryStream* in);
	void ListCurrentGames(InputMemoryStream* in);

public:
	GameManager() {}
	~GameManager();

	void CheckArray();

	void ClientControl(TcpSocket* serverSock);
	void CalculateOrganQuantity();

	bool Update();
	void Start();

	void SetReady();

	void CreateGame(TcpSocket* serverSock);

	void SetPort(unsigned int _port) { localPort = _port; };
	void SetEndRound(bool _round) { *endRound = _round; }
	int GetPlayersNum() { return socks->size() + 1; };

	bool GetReady() { return ready; }
	int GetPlayersReady() { return *playersReady; }
};