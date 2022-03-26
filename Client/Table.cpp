
#include <iostream>
#include "Table.h"


Table::Table()
{
}

Table::~Table()
{
}

void Table::ShowTable()
{
	for (size_t j = 0; j < table.size(); j++)
	{
		for (size_t i = 0; i < table[j].size(); i++)
		{
			std::string cardType;
			std::string organType;
			std::string threatmentType;

			switch ((int)table[j][i]->cardType)
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

			switch ((int)table[j][i]->organType)
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

			switch ((int)table[j][i]->treatmentType)
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

			std::cout << "Card: " + std::to_string(i) + " cardType: " + cardType + " OrganType: " + organType + " ThreatmentType: " + threatmentType << " Virus Quantity: " << table[j][i]->virusQuantity << std::endl;
		}
	}

}
