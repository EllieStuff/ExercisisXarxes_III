#pragma once
#include "GameManager.h"

class SceneManager
{
	enum class State {INIT, GAME, END};
	State gameState;

	std::vector<CriticalMessages> criticalMessages;

	GameManager* game;

	void UpdateInit();
	void ReceiveMessages();
	void EnterGame();
	void UpdateGame();

public:
	SceneManager();
	~SceneManager();

	void Update();
};