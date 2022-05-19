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

	int tries;

	void GenerateSALT(int &value, int numOfBytes);

public:
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

	std::string GetName() { return name; }

	void ResetTimeOut() { timeout = clock(); }

	bool searchingForMatch;
	int matchID;
	int playerQuantity;
};