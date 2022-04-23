#pragma once
#include "GameManager.h"

enum class Status;
class IpAdress;

class SceneManager
{
	enum class State {INIT, GAME, END};
	State gameState;

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