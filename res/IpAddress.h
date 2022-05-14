#pragma once
#include <SFML/Network.hpp>

class IpAddress
{
	sf::IpAddress address;

public:
	IpAddress();
	IpAddress(sf::IpAddress _address);
	IpAddress(std::string _address);
	~IpAddress();

	void SetAddress(std::string _address);

	sf::IpAddress* GetAddress() { return &address; }

	std::string GetLocalAddress();
	std::string GetPublicAddress();
};

inline bool operator == (IpAddress _first, IpAddress _second) 
{
	return *_first.GetAddress() == *_second.GetAddress();
}