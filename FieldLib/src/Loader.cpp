#include "Filer.h"

#include "nlohmann/json.hpp"
using json = nlohmann::json;







int Filer::LoadN(std::string Type,int number,std::string kvalue) 
{
	std::string fType;
	if (Type == "L")
		fType = "Lines";
	else if (Type == "H")
		fType = "Hashes";
	else if (Type == "T")
		fType = "Texts";


	std::string Line = std::to_string(number);

	std::ifstream f("default.json");
	json data = json::parse(f);
	int out = data[fType][Line][kvalue];
	return out;
}

std::string Filer::LoadS(std::string Type, int number, std::string kvalue)
{
	std::string fType;
	if (Type == "T")
		fType = "Texts";


	std::string Line = std::to_string(number);

	std::ifstream f("default.json");
	json data = json::parse(f);
	std::string out = data[fType][Line][kvalue];
	return out;
}

bool Filer::ContainsT(std::string Type,int Num)
{
	std::string fType = "Lines";
	if (Type == "L")
		fType = "Lines";
	else if (Type == "H")
		fType = "Hashes";
	else if (Type == "T")
		fType = "Texts";

	std::string Number = std::to_string(Num);

	std::ifstream f("default.json");
	json data = json::parse(f);
	int out = data[fType].contains(Number);
	return out;
}