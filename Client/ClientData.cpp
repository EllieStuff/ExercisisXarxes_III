#include "ClientData.h"

void ClientData::GenerateSALT(int& value, int numOfBytes = 4)
{
	value = rand() & 0xff;
	for (size_t i = 1; i < numOfBytes; i++)
	{
		value |= (rand() & 0xff) << (i * 8);
	}
}

ClientData::ClientData(unsigned short _port, IpAddress _address, std::string _name): port(_port), address(_address), name(_name)
{
	GenerateSALT(clientSALT);
}

int ClientData::CalculateChallenge(int _salt)
{
	return _salt & serverSALT;
}

void ClientData::SetClientID(int _id)
{
	clientID = _id;
}

void ClientData::SetServerSalt(int _salt)
{
	serverSALT = _salt;
}
