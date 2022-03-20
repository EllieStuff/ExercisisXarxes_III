#pragma once
#include "../res/Utils.h"
#include <vector>

class PlayerHand
{
public:
	std::vector<Card*> hand;

	PlayerHand();
	~PlayerHand();

	void ListCards();

	void AddCard(Card* newCard);
	void PlayCard(int _cardIdx);
};