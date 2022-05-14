#pragma once
#include "GameManager.h"
#include <thread>

class SceneManager
{
	enum class State {INIT, GAME, END};
	State gameState;
	bool* connected;
	int packetId;

	std::map<Commands, CriticalMessages>* criticalMessages;

	GameManager* client;

	void UpdateInit();

	void EnterGame();
	void UpdateGame();
	void ReceiveMessages();
	void SavePacketToTable(Commands _packetId, OutputMemoryStream* out, std::time_t time);
	void CheckMessageTimeout();

	void MessageReceived(Commands _message);

public:
	SceneManager();
	~SceneManager();

	void Update();
};