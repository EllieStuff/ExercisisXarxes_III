#pragma once
#include "../res/Utils.h"
#include <vector>
#include <mutex>

class Deck
{
	std::mutex mtx;
public:
	std::vector<Card*> deck;

	Deck();
	~Deck();

	
};