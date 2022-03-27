#pragma once
#include "Player.h"
#include <mutex>
#include <list>
#include "../res/InputMemoryStream.h"
#include "../res/TcpSocket.h"
#include "../res/TcpListener.h"
#include "../res/Selector.h"

class TcpSocket;

class GameManager
{
	TcpListener listener;
	unsigned int localPort = 0;

	int* currentGameID = new int(-1);

	std::vector<TcpSocket*>* socks = new std::vector<TcpSocket*>();
	Player* player = new Player();
	Deck* deck = new Deck();
	Table* table = new Table();
	int* currentTurn = new int(0);
	bool* endRound = new bool(false);
	std::mutex mtx;
	int gameMaxSize = 4;

	bool ready = false;
	int* playersReady = new int(0);

	std::vector<Pair_Organ_Player> playerTurnOrder;

	void UpdateTurn(bool plus);
	void ReceiveMessages(InputMemoryStream in1);
	void AcceptConnections(Selector* selector, TcpListener* listener);
	void SendReady();
	void SetListener();

	void SendPassword(OutputMemoryStream* out);

public:
	GameManager();
	~GameManager();

	void ListEnemiesWithTheirCards();

	void ClientControl(TcpSocket* serverSock);
	bool Threatment();

	bool PlaceInfection();

	bool VaccineOrgan();

	void CheckArray();

	void CalculateOrganQuantity();
	void ConnectP2P(Selector* selector, InputMemoryStream* in);

	bool Update();
	void Start();

	void SetReady();

	void CreateGame(OutputMemoryStream* out);
	void ListCurrentGames(InputMemoryStream* in);
	void JoinGame(OutputMemoryStream* out, bool& _aborted);

	void SetPort(unsigned int _port) { localPort = _port; };
	void SetEndRound(bool _round) { *endRound = _round; }
	int GetPlayersNum() { return socks->size() + 1; };
	int GetGameSize() { return gameMaxSize; }

	bool GetReady() { return ready; }
	int GetPlayersReady() { return *playersReady; }
};