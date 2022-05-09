#pragma once
#include "GameManager.h"
#include <thread>

class SceneManager
{
	enum class State {INIT, GAME, END};
	State gameState;
	bool* connected;

	GameManager* client;

	void UpdateInit();

	void EnterGame();
	void UpdateGame();
	void ReceiveMessages();

public:
	SceneManager();
	~SceneManager();

	void Update();
};