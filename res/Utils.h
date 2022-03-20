#pragma once
#include <string>
#include <vector>

enum class Status { DONE, NOT_READY, PARTIAL, DISCONNECTED, ERROR };

enum class Commands {CREATE_GAME = 1, GAME_LIST, JOIN_GAME, INCORRECT_PWD, ORGAN_QUANTITY, UPDATE_TURN, END_ROUND};

struct PeerAddress {
	std::string ip;
	int port;
};

struct Game
{
	std::vector<PeerAddress> peers;

	int gameSize, gameId;
	std::string pwd = "", gameName = "";
};

struct Pair_Organ_Player
{
	int playerID, numOrgans;

	Pair_Organ_Player(int _playerID, int _numOrgans) : playerID(_playerID), numOrgans(_numOrgans)
	{

	}

	bool operator>(int _numOrgans)
	{
		return numOrgans > _numOrgans;
	}

	bool operator<(int _numOrgans)
	{
		return numOrgans < _numOrgans;
	}

	bool operator>=(int _numOrgans)
	{
		return numOrgans >= _numOrgans;
	}

	bool operator<=(int _numOrgans)
	{
		return numOrgans <= _numOrgans;
	}
};

struct Card {
public:
	enum class CardType { ORGAN, MEDICINE, VIRUS, TREATMENT, COUNT, NONE };
	enum class OrganType { STOMACH, BRAIN, SKELETON, HEART, NONE };
	enum class TreatmentType { INFECTION, ROBER, TRANSPLANT, LATEX_GLOVES, MEDICAL_ERROR, NONE };

	CardType cardType;
	OrganType organType;
	TreatmentType treatmentType;
	bool isWildcard = false;

	Card(CardType _type, OrganType _organ, bool _isWildcard = false) 
	{
		cardType = _type;
		organType = _organ;
		isWildcard = _isWildcard;
		treatmentType = TreatmentType::NONE;
	}
	Card(TreatmentType _treatment, bool _isWildcard = false)
	{
		organType = OrganType::NONE;
		cardType = CardType::TREATMENT;
		treatmentType = _treatment;
		isWildcard = _isWildcard;
	}
};

struct VirusData {
	int playerId;
	int currTurn;
	bool hasStolen;	// Nota: passar cartes al principi de la principi
	// ToDo?
};


inline void ListCards()
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

