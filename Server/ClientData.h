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
	std::vector<float> rttLog;

	int saltTries = 0;

	void GenerateSALT(int &value, int numOfBytes);

public:

	bool searchingForMatch;
	int matchID;
	int playerQuantity;

	ClientData() {}
	ClientData(unsigned short _port, IpAddress _address, std::string _name, int _salt);

	bool CheckChallengeResult(int _salt);

	void SetClientID(int _id) { clientID = _id; }
	void AddClientRtt(float _rtt) { rttLog.push_back(_rtt); }

	IpAddress GetAddress() { return address; }

	unsigned short GetPort() { return port; }

	int GetServerSalt() { return serverSALT; }
	int GetClientSalt() { return clientSALT; }
	float GetClientRtt(int _rttIdx) { return rttLog[_rttIdx]; }
	int GetRttSize() { return rttLog.size(); }
	float GetRttAvarage();

	std::string GetName() { return name; }

	void ResetTimeOut() { timeout = clock(); }

	int GetTries() { return saltTries; };
	void AddTry() { saltTries++; }
};