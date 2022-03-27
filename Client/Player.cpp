#include "Player.h"
#include <iostream>

Player::Player()
{
}

Player::~Player()
{
}

void Player::ReceiveCards(int _quantity, Deck* _deck)
{
	if (_quantity <= 0) return;

	for (size_t i = 0; i < _quantity; i++)
	{
		hand.hand.push_back(_deck->deck.back());
		_deck->deck.pop_back();
	}
}

bool Player::PlaceCard(int _pos, Card::CardType _cardType, Table* _table, Deck* _deck)
{
	if (hand.hand[_pos]->cardType == _cardType)
	{
		_table->table[id].push_back(hand.hand[_pos]);
		hand.hand.erase(hand.hand.begin() + _pos);
		return true;
	}
	else
	{
		std::cout << "Invalid Card!!!!!" << std::endl;
		return false;
	}
}
