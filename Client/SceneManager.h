#pragma once
#include "GameManager.h"
#include <thread>
#include <mutex>

class SceneManager
{
	struct GamePlayerInfo
	{
		int currInputX = 0;
		int currInputY = 0;

		float posX;
		float posY;

		float oldX = 0;
		float oldY = 0;

		GamePlayerInfo(float _posX, float _posY)
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

	enum class State {INIT, GAME, END, EXIT};
	State* gameState;
	bool* connected;
	int packetId;
	int saltTries;
	int matchID;

	bool left = false, right = false, up = false, down = false;

	std::mutex mtx;

	bool* pong;
	bool* match;

	bool startedThreadsAlready;

	std::map<Commands, CriticalMessages>* criticalMessages;
	std::vector<std::pair<int, int>> accumulatedMessages;

	GameManager* client;

	void UpdateInit();

	void EnterGame();
	void UpdateGame();
	void ReceiveMessages();
	void SavePacketToTable(Commands _packetId, OutputMemoryStream* out, std::time_t time);
	void CheckMessageTimeout();
	void Ping(float rttKey);

	void MessageReceived(Commands _message);

public:
	SceneManager();
	~SceneManager();

	void Update();
};