#pragma once
#include "GameManager.h"

class SceneManager
{
	enum class State {INIT, GAME, END};
	State gameState;

	std::map<int, std::map<Commands, CriticalMessages>*>* criticalMessages;

	GameManager* game;

	void SavePacketToTable(Commands _packetId, OutputMemoryStream* out, std::time_t time, int _id);

	void UpdateInit();
	void ReceiveMessages();
	void EnterGame();
	void UpdateGame();
	void CheckMessageTimeout();
	void MessageReceived(Commands message, int _id, float _rttKey);
	void PrintRttAvarage();

public:
	SceneManager();
	~SceneManager();

	void Update();
};