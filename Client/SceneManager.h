#pragma once
#include "GameManager.h"
#include <thread>
#include <mutex>

class SceneManager
{
	struct GamePlayerInfo
	{
		float posX;
		float posY;

		float oldX;
		float oldY;

		GamePlayerInfo(int _posX, int _posY) 
		{
			posX = _posX;
			posY = _posY;
		}

		void SetPlayerPos(float _posX, float _posY)
		{
			posX = _posX;
			posY = _posY;
		}

		void SetOldPlayerPos(float _posX, float posY) 
		{
			oldX = _posX;
			oldY = posY;
		}
	};

	std::map<int, GamePlayerInfo>* players;

	enum class State {INIT, GAME, END};
	State gameState;
	bool* connected;
	int packetId;
	int saltTries;
	int matchID;

	std::mutex mtx;

	bool* pong;
	bool* match;

	bool startedThreadsAlready;

	std::map<Commands, CriticalMessages>* criticalMessages;

	GameManager* client;

	void UpdateInit();

	void EnterGame();
	void UpdateGame();
	void ReceiveMessages();
	void SavePacketToTable(Commands _packetId, OutputMemoryStream* out, std::time_t time);
	void CheckMessageTimeout();
	void Ping();

	void MessageReceived(Commands _message);

public:
	SceneManager();
	~SceneManager();

	void Update();
};