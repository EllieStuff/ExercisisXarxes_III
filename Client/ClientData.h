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

	void GenerateSALT(int &value, int numOfBytes);

public:
	ClientData() {}

	int CalculateChallenge(int _salt);

	void Start(unsigned short _port, IpAddress _address, std::string _name);

	void SetClientID(int _id);
	void SetServerSalt(int _salt);
	void SetClientSalt(int _salt) { clientSALT = _salt; };

	int GetClientID() { return clientID; }

	IpAddress GetAddress() { return address; }

	unsigned short GetPort() { return port; }

	int GetServerSalt() { return serverSALT; }
	int GetClientSalt() { return clientSALT; }

	void SetSalt(int salt) { clientSALT = salt; };

	std::string GetName() { return name; }

	void ResetTimeOut() { timeout = clock(); }
};