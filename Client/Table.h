#pragma once
#include <vector>
#include "../res/Utils.h"


class Table 
{
private:

public:
	std::vector<std::vector<Card*>> table;

	Table();
	~Table();

	void ShowTable();
};