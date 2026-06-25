#pragma once
#include <string>

class Ingredient
{
public:
	Ingredient();
	Ingredient(std::string name, double quantity);

	int GetId() const;
	std::string GetName() const;
	double GetLevel() const;
	bool ReduceLevel(double amount);

	bool operator==(const Ingredient& other) const;
	void operator=(const Ingredient& other);
private:
	std::string name;
	double quantity;
};

