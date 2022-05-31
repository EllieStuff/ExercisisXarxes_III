#pragma once
#include "GameManager.h"
#include <mutex>

class SceneManager
{
	const int POS_MARGIN = 10;

	std::mutex mtx;
	enum class State {INIT, GAME, END};
	State* gameState;
	int matchID = 0;

	std::map<int, std::map<Commands, CriticalMessages>*>* criticalMessages;
	std::map<int, std::vector<std::pair<int, ClientData*>>>* rooms;
	std::vector<std::pair<int, ClientData*>>* searchingPlayers;

	GameManager* game;

	void SavePacketToTable(Commands _packetId, OutputMemoryStream* out, std::time_t time, int _id);

	void UpdateInit();
	void ReceiveMessages();
	void EnterGame();
	void UpdateGame();
	void CheckMessageTimeout();
	void MessageReceived(Commands message, int _id, float _rttKey);
	void SearchMatch(int _id, int _matchID, bool _createOrSearch);
	void UpdateGameInfo(int _gameID, int hostID);

	void CheckRooms();
	void MatchMaking();

	void DisconnectClient(int _id);

	void ExitThread();
public:
	SceneManager();
	~SceneManager();

	void Update();
};