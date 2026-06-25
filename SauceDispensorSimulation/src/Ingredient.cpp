#include "Ingredient.h"

Ingredient::Ingredient()
{
	name = "default";
	quantity = -1;
}

Ingredient::Ingredient(std::string name, double quantity)
	:name(name), quantity(quantity)
{}

std::string Ingredient::GetName() const
{
	return name;
}

double Ingredient::GetLevel() const
{
	return quantity;
}

bool Ingredient::operator==(const Ingredient& other) const
{
	return this->name == other.name;
}

void Ingredient::operator=(const Ingredient& other)
{
	this->name = other.name;
	this->quantity = other.quantity;
}
