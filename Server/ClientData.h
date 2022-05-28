#pragma once
#include "../res/IpAddress.h"
#include <time.h>

class ClientData
{
	unsigned short port;
	IpAddress address;
	std::string name;
	int clientSALT, serverSALT, clientID;
	clock_t timeout;
	std::map<float, float> rttLog;

	int saltTries = 0;

	int posX = 0;
	int posY = 0;

	void GenerateSALT(int &value, int numOfBytes);

public:

	bool disconnected;
	bool searchingForMatch;
	int matchID;
	int playerQuantity;

	ClientData() {}
	ClientData(unsigned short _port, IpAddress _address, std::string _name, int _salt);

	bool CheckChallengeResult(int _salt);

	void SetClientID(int _id) { clientID = _id; }
	void SetClientRtt(float _rttKey, float _realRtt) { rttLog[_rttKey] = _realRtt; }

	IpAddress GetAddress() { return address; }

	unsigned short GetPort() { return port; }

	int GetServerSalt() { return serverSALT; }
	int GetClientSalt() { return clientSALT; }
	float GetClientRtt(float _rttKey) { return rttLog[_rttKey]; }

	int GetId() { return clientID; }
	std::string GetName() { return name; }

	void ResetTimeOut() { timeout = clock(); }

	void SetPosition(int _posX, int _posY) { posX = _posX; posY = _posY; }
	int GetXPos() { return posX; }
	int GetYPos() { return posY; }

	int GetTries() { return saltTries; };
	void AddTry() { saltTries++; }
};