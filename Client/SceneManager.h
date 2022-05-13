#pragma once
#include "GameManager.h"
#include <thread>

class SceneManager
{
	enum class State {INIT, GAME, END};
	State gameState;
	bool* connected;
	int packetId;

	std::map<int, CriticalMessages> criticalMessages;

	GameManager* client;

	void UpdateInit();

	void EnterGame();
	void UpdateGame();
	void ReceiveMessages();
	void SavePacketToTable(OutputMemoryStream* out, std::time_t time);
	void CheckMessageTimeout();

public:
	SceneManager();
	~SceneManager();

	void Update();
};