#pragma once
#include "SimulationExecutive.h"
#include "LoggerLib.h"
#include "Bowl.h"

class Conveyor;
class Carousel;


// ================ SIMULATED ROBOT ===============

class SauceDispensorRobot
{
public:
	SauceDispensorRobot(std::vector<Bowl*> orders, Time carousel_taskrate, Time conveyor_taskrate, Time dispensor_taskrate);
	void AddOrders(std::vector<Bowl*> orders);

private:
	Conveyor* conveyor;
	Carousel* carousel;
	static Logger* logger;

	std::vector<Bowl> incoming_orders;
};


// ================ SIMULATED HARDWARE ===============

class DispensingHead
{
public:
	DispensingHead(int id, Carousel* carousel, Time taskrate);

	void SetIngredient(Ingredient* ingredient);
	void SetAmountToDispense(double amount_to_dispense);
	int GetLocation();

private:
	// simulated control [Dispense solenoid actuation]
	void Dispense(Conveyor* conveyor);

	Ingredient* active_ing;
	double amount;
	int dispenseLocation; // The slot on the conveyor where the dispensing head is
	std::string location;
	Carousel* carousel; // The carousel that the dispensing head is attached to
	Time taskrate;
	static Logger* logger;

	// ==== EVENT ACTIONS ====
	class DispenseEA;
};

class Carousel
{
public:
	Carousel(int numHeads, Time taskrate, Time dispensor_taskrate);

	// simulated sensors [Sauce level per cartridge, Conveyor Position Feedback]
	double GetSauceLevel(int index);
	double* GetSauceLevels();

private:
	void UpdateDispensorLocations(int offset);

	// simulated control [Carousel rotation]
	void Advance();

	DispensingHead** dispensingHeads; // Heads attached to the carousel
	Ingredient* ingredientCartridges; // what ingredients are in the dispensing cartridges
	Ingredient* dispensorIngredients; // [upstream ingredient, downstream ingredient]
	Time taskrate;
	static Logger* logger;

	// simulated sensor values
	int positionOffset;
	double* sauceLevels;

	// ==== EVENT ACTIONS ====
	class AdvanceEA;
};

class Conveyor
{
public:
	Conveyor(int size, Time taskrate);

	void SetQueue(std::vector<Bowl*> orders);
	void AddOrdersToQueue(std::vector<Bowl*> orders);

	// simulated control [Conveyor belt actuation, Dispensor ingredient on bowl on conveyor]
	void Advance();
	void DispenseSauceOn(int index, Ingredient* ingredient);

	// simulated sensors [Bowl weight per position]
	int GetBowlWeight(int index);
	std::vector<int> GetBowlWeights();

private:
	int slots;
	Bowl** bowls; // Bowls on the conveyor
	Time taskrate;
	static Logger* logger;
	std::vector<Bowl*> queue;

	// ==== EVENT ACTIONS ====
	class AdvanceEA;
};