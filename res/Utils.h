#pragma once
#include <string>
#include <vector>
#include "IpAddress.h"

//Usar puerto inicial para el cliente y si el status de bindear a este es negativo, incrementar este puerto inicial
const unsigned short Client_Initial_Port = 500;

const int MAX_TRIES = 5;

const unsigned short Server_Port = 5000;
const std::string Server_Ip = "127.0.0.1";

enum class Status { DONE, NOT_READY, PARTIAL, DISCONNECTED, ERROR };

enum class Commands {
	DEFAULT, WELCOME, HELLO, PLAYER_ID, SALT, CHALLENGE, ACK_WELCOME, PING_PONG, SEARCH_MATCH, MATCH_FOUND,

	COUNT
};

struct CriticalMessages {
	IpAddress ip;
	unsigned short port;
	std::time_t startTime;
	std::time_t currentTime;
	OutputMemoryStream* message;

	CriticalMessages() {}
	CriticalMessages(IpAddress _ip, unsigned short _port, std::time_t _timeout, OutputMemoryStream* _message)
	{
		ip = _ip;
		port = _port;
		startTime = _timeout;
		message = _message;
	};


};

//Codigo antiguo de TCP

//enum class Commands {
//	DEFAULT = 0, LOG, GAME_ID, PLAYER_ID, INCORRECT_NAME, INCORRECT_ID, CORRECT_PWD, SENDCARD, SHOWCARDS, LISTCARDS, CREATE_GAME, GAME_LIST,
//	JOIN_GAME, INCORRECT_PWD, PWD_CHECK, PROTECTED, PLAYER_READY, PLAYER_LIST, ORGAN_QUANTITY, UPDATE_TURN, END_ROUND, PLACE_ORGAN, I_WON,
//	PLACE_INFECTION, PLACE_MEDICINE, DISCARD_CARD, PLACE_TREATMENT, LIST_CARDS, SHOW_CARDS, PWD_FILTER, NUM_PLAYERS_FILTER, NO_FILTER, SEND_TO_ALL_PLAYERS, ACTIVATE_FILTER, COUNT
//};
//struct PeerAddress {
//	std::string ip;
//	int port;
//};
//
//struct Game
//{
//	std::vector<PeerAddress> peers;
//
//	int gameMaxSize = 3, gameId;
//	std::string pwd = "", gameName = "";
//};
//
//struct Pair_Organ_Player
//{
//	int playerID = 0, numOrgans = 0;
//
//	Pair_Organ_Player(int _playerID, int _numOrgans) : playerID(_playerID), numOrgans(_numOrgans)
//	{
//
//	}
//
//	bool operator>(Pair_Organ_Player _other)
//	{
//		return numOrgans > _other.numOrgans;
//	}
//
//	bool operator<(Pair_Organ_Player _other)
//	{
//		return numOrgans < _other.numOrgans;
//	}
//
//	bool operator>=(Pair_Organ_Player _other)
//	{
//		return numOrgans > _other.numOrgans || (numOrgans == _other.numOrgans && playerID > _other.playerID);
//	}
//
//	bool operator<=(Pair_Organ_Player _other)
//	{
//		return numOrgans < _other.numOrgans || (numOrgans == _other.numOrgans && playerID < _other.playerID);
//	}
//};
//
//inline bool ComparePlayers(Pair_Organ_Player _first, Pair_Organ_Player _second)
//{
//	return _first <= _second;
//}
//
//struct Card {
//public:
//	enum class CardType { ORGAN, MEDICINE, VIRUS, TREATMENT, COUNT, NONE };
//	enum class OrganType { STOMACH, BRAIN, SKELETON, HEART, NONE };
//	enum class TreatmentType { INFECTION, ROBER, TRANSPLANT, LATEX_GLOVES, MEDICAL_ERROR, NONE };
//
//	CardType cardType;
//	OrganType organType;
//	TreatmentType treatmentType;
//	bool isWildcard = false;
//	int virusQuantity = 0;
//	int VaccineQuantity = 0;
//
//	Card(CardType _type, OrganType _organ, bool _isWildcard = false) 
//	{
//		cardType = _type;
//		organType = _organ;
//		isWildcard = _isWildcard;
//		treatmentType = TreatmentType::NONE;
//	}
//	Card(TreatmentType _treatment, bool _isWildcard = false)
//	{
//		organType = OrganType::NONE;
//		cardType = CardType::TREATMENT;
//		treatmentType = _treatment;
//		isWildcard = _isWildcard;
//	}
//};
//
//struct VirusData {
//	int playerId;
//	int currTurn;
//	bool hasStolen;	// Nota: passar cartes al principi de la principi
//	// ToDo?
//};

//