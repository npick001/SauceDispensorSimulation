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
	SauceDispensorRobot(std::vector<Bowl*> orders, std::vector<Ingredient*> ingredients, Time carousel_taskrate, Time conveyor_taskrate, Time dispensor_taskrate);
	void AddOrders(std::vector<Bowl*> orders);

private:
	Conveyor* conveyor;
	Carousel* carousel;

	std::vector<Bowl> incoming_orders;
};


// ================ SIMULATED HARDWARE ===============

class DispensingHead
{
public:
	DispensingHead(int head_id, Carousel* carousel, Conveyor* conv, Time taskrate);

	void SetIngredient(Ingredient* ingredient);
	void SetAmountToDispense(double amount_to_dispense);
	int GetLocation();
	int GetId();

private:
	// simulated control [Dispense solenoid actuation]
	void Dispense();

	int id;
	Ingredient* active_ing;
	double amount;
	int dispenseLocation; // The slot on the conveyor where the dispensing head is
	std::string location;
	Carousel* carousel; // The carousel that the dispensing head is attached to
	Conveyor* conveyor; // The conveyor that the dispensing head is dispensing onto
	Time taskrate;
	static Logger* logger;

	// ==== EVENT ACTIONS ====
	class DispenseEA;
};

class Carousel
{
public:
	Carousel(int numHeads, Time taskrate, Time dispensor_taskrate, std::vector<Ingredient*> ingredients, Conveyor* conv);
	void ReduceSauceLevel(int head, double amount);

	// simulated sensors [Sauce level per cartridge, Conveyor Position Feedback]
	double GetSauceLevel(int index);
	std::vector<double> GetSauceLevels();

private:
	void SetIngredientCartridges(std::vector<Ingredient*> ingredients);
	void UpdateDispensorLocations(int offset);

	// simulated control [Carousel rotation]
	void Advance();

	std::vector<DispensingHead*> dispensingHeads; // Heads attached to the carousel
	std::vector<Ingredient*> ingredientCartridges; // what ingredients are in the dispensing cartridges
	std::vector<Ingredient*> dispensorIngredients; // [upstream ingredient, downstream ingredient]
	Time taskrate;
	static Logger* logger;

	// simulated sensor values
	int positionOffset;
	std::vector<double> sauceLevels;

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
	double GetBowlWeight(int index);
	std::vector<double> GetBowlWeights();

private:
	int slots;
	std::vector<std::unique_ptr<Bowl>> bowls; // Bowls on the conveyor
	Time taskrate;
	static Logger* logger;
	std::vector<std::unique_ptr<Bowl>> queue;

	// ==== EVENT ACTIONS ====
	class AdvanceEA;
};