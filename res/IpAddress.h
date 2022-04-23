#pragma once
#include <SFML/Network.hpp>
#include "InputMemoryStream.h"
#include "OutputMemoryStream.h"
#include "Utils.h"

class IpAddress
{
	sf::IpAddress address;

public:
	IpAddress();
	~IpAddress();

	sf::IpAddress* GetAddress() { return &address; }

	std::string GetLocalAddress();
	std::string GetPublicAddress();
};