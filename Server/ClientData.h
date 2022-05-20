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
	std::vector<double> rttLog;

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
	void AddClientRtt(double _rttTime) { rttLog.push_back(_rttTime); }

	IpAddress GetAddress() { return address; }

	unsigned short GetPort() { return port; }

	int GetServerSalt() { return serverSALT; }
	int GetClientSalt() { return clientSALT; }
	float GetRttAvarage();

	void CleanRttLog() { rttLog.clear(); }

	std::string GetName() { return name; }

	void ResetTimeOut() { timeout = clock(); }

	int GetTries() { return saltTries; };
	void AddTry() { saltTries++; }
};