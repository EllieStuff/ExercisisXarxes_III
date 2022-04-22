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

	std::string GetLocalAddress();
	std::string GetPublicAddress();
};