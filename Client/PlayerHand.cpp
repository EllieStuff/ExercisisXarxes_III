#include "PlayerHand.h"

PlayerHand::PlayerHand()
{
}

PlayerHand::~PlayerHand()
{
	for (size_t i = 0; i < hand.size(); i++)
	{
		delete hand[i];
	}
	hand.clear();
}

void PlayerHand::AddCard(Card* newCard)
{
}

void PlayerHand::PlayCard(int _cardIdx)
{
}
