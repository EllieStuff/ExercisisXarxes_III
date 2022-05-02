#pragma once
#include "../res/IpAddress.h"
#include <time.h>

class ClientData
{
	unsigned short port;
	IpAddress address;
	std::string name;
	int clientSALT, serverSALT, clientID;

	void GenerateSALT(int &value, int numOfBytes);

public:
	ClientData() {}
	ClientData(unsigned short _port, IpAddress _address, std::string _name);

	int CalculateChallenge(int _salt);

	void SetClientID(int _id);
	void SetServerSalt(int _salt);

	IpAddress GetAddress() { return address; }

	unsigned short GetPort() { return port; }

	int GetSalt() { return clientSALT; };

	std::string GetName() { return name; }
};