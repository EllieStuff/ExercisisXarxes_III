#pragma once
#include "PlayerHand.h"
#include "Deck.h"
#include "Table.h"
#include <vector>



class Player
{
private:

public:
	PlayerHand hand;
	int id;

	Player();
	~Player();

	void ReceiveCards(int _quantity, Deck* _deck);
	bool PlaceCard(int _pos, Card::CardType _cardType, Table* _table, Deck* _deck);

};