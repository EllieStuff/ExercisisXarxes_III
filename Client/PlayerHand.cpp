#include "PlayerHand.h"
#include <iostream>

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

void PlayerHand::ListCards()
{
	for (size_t i = 0; i < hand.size(); i++)
	{
		std::string cardType;
		std::string organType;
		std::string threatmentType;

		switch ((int)hand.at(i)->cardType)
		{
		case 0:
			cardType = "ORGAN";
			break;
		case 1:
			cardType = "MEDICINE";
			break;
		case 2:
			cardType = "VIRUS";
			break;
		case 3:
			cardType = "TREATMENT";
			break;
		}

		switch ((int)hand.at(i)->organType)
		{
		case 0:
			organType = "STOMACH";
			break;
		case 1:
			organType = "BRAIN";
			break;
		case 2:
			organType = "SKELETON";
			break;
		case 3:
			organType = "HEART";
			break;
		case 4:
			organType = "NONE";
			break;
		}

		switch ((int)hand.at(i)->treatmentType)
		{
		case 0:
			threatmentType = "INFECTION";
			break;
		case 1:
			threatmentType = "ROBER";
			break;
		case 2:
			threatmentType = "TRANSPLANT";
			break;
		case 3:
			threatmentType = "LATEX_GLOVES";
			break;
		case 4:
			threatmentType = "MEDICAL_ERROR";
			break;
		case 5:
			threatmentType = "NONE";
			break;
		}
		if (cardType._Equal("TREATMENT"))
			std::cout << "Card: " << i << " cardType: " << cardType << " ThreatmentType: " << threatmentType << std::endl;
		else
			std::cout << "Card: " << i << " cardType: " << cardType << " OrganType: " << organType << std::endl;
	}
}

void PlayerHand::AddCard(Card* newCard)
{
}

void PlayerHand::PlayCard(int _cardIdx)
{
}
