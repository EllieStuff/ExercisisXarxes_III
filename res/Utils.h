#pragma once
#include <string>


enum class Status { DONE, NOT_READY, PARTIAL, DISCONNECTED, ERROR };

struct PeerAddress {
	std::string ip;
	int port;
};


struct Card {
private:
	enum class CardType { ORGAN, MEDICINE, VIRUS, TREATMENT };
	enum class OrganType { STOMACH, BRAIN, SKELETON, HEART, NONE };
	enum class TreatmentType { INFECTION, ROBER, TRANSPLANT, LATEX_GLOVES, MEDICAL_ERROR, NONE };

public:
	CardType cardType;
	OrganType organType;
	TreatmentType treatmentType;
	bool isWildcard = false;
};

struct VirusData {
	int playerId;
	int currTurn;
	bool hasStolen;	// Nota: passar cartes al principi de la principi
	// ToDo?
};

