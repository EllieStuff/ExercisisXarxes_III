#pragma once
#include "GameManager.h"
#include <thread>
#include <mutex>

class SceneManager
{
	enum class State {INIT, GAME, END};
	State gameState;
	bool* connected;
	int packetId;

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