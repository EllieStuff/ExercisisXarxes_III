#pragma once
#include "../res/IpAddress.h"
#include <time.h>

class ClientData
{
	unsigned short port;
	IpAddress address;
	std::string name;
	int clientSALT, serverSALT, clientID;


public:
	ClientData() {}
	ClientData(unsigned short _port, IpAddress _address, std::string _name);

	int CalculateChallenge(int _salt);
	void GenerateSALT(int &value, int numOfBytes);

	void SetClientID(int _id);
	void SetServerSalt(int _salt);
	void SetClientSalt(int _salt) { clientSALT = _salt; };

	int GetClientID() { return clientID; }

	IpAddress GetAddress() { return address; }

	unsigned short GetPort() { return port; }

	int GetSalt() { return clientSALT; };

	void SetSalt(int salt) { clientSALT = salt; };

	int CreateSALT();

	std::string GetName() { return name; }

};