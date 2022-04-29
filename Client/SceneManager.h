#pragma once
#include "GameManager.h"

class SceneManager
{
	enum class State {INIT, GAME, END};
	State gameState;

	GameManager* client;

	void UpdateInit();

	void EnterGame();
	void UpdateGame();

public:
	SceneManager();
	~SceneManager();

	void Update();
};