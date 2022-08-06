#pragma once

#include <iostream>
#include <fstream>

namespace Filer {
	void Save();
	int LoadN(std::string Type, int number, std::string kvalue);
	std::string LoadS(std::string Type, int number, std::string kvalue);
	bool ContainsT(std::string Type, int Num);
}