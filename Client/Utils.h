#pragma once
#include <string>


enum class Status { DONE, NOT_READY, PARTIAL, DISCONNECTED, ERROR };


struct PeerAddress {
	std::string ip;
	unsigned short port;
};

