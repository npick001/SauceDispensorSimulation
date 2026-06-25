#include "SauceDispensorRobot.h"

Logger* SauceDispensorRobot::logger = Logger::GetInstance("logs", "sauce_dispensor_log.txt", 31);
Logger* DispensingHead::logger = Logger::GetInstance("logs", "dispensing_head_log.txt", 31);
Logger* Carousel::logger = Logger::GetInstance("logs", "carousel_log.txt", 31);

// ==== EVENT ACTIONS ====

// ==== DISPENSING HEAD ====
class DispensingHead::DispenseEA : public EventAction
{
public:
	DispenseEA(DispensingHead* d, Conveyor* c)
		: dispensor(d), conveyor(c)
	{}

	void Execute() {
		dispensor->Dispense(conveyor);
	}

private:
	DispensingHead* dispensor;
	Conveyor* conveyor;
};

// ==== CAROUSEL ====
class Carousel::AdvanceEA : public EventAction
{
public:
	AdvanceEA(Carousel* c)
		: carousel(c)
	{}

	void Execute() {
		carousel->Advance();
	}

private:
	Carousel* carousel;
};

// ==== CONVEYOR ====
class Conveyor::AdvanceEA : public EventAction
{
public:
	AdvanceEA(Conveyor* c)
		: conveyor(c)
	{}

	void Execute() {
		conveyor->Advance();
	}

private:
	Conveyor* conveyor;
};


// ================ SIMULATED ROBOT ===============

SauceDispensorRobot::SauceDispensorRobot(std::vector<Bowl*> orders, Time carousel_taskrate, Time conveyor_taskrate, Time dispensor_taskrate)
{
	carousel = new Carousel(2, carousel_taskrate, dispensor_taskrate);
	conveyor = new Conveyor(6, conveyor_taskrate);
	conveyor->SetQueue(orders);
}

void SauceDispensorRobot::AddOrders(std::vector<Bowl*> orders)
{
	conveyor->AddOrdersToQueue(orders);
}

// ================ SIMULATED HARDWARE ===============
// ==== DISPENSING HEAD ====

DispensingHead::DispensingHead(int id, Carousel* c, Time rate)
{
	carousel = c;
	active_ing = nullptr;
	amount = -1.0;
	taskrate = rate;

	if (id == 0) {
		dispenseLocation = 1; // slot 1 on the conveyor based on the image
		location = "Left";
		logger->Log(LogLevel::INFO, "Left Dispensor instantiated and location added.");
	}
	else if (id == 1) {
		dispenseLocation = 4; // slot 4 on the conveyor based on the image
		location = "Right";
		logger->Log(LogLevel::INFO, "Right Dispensor instantiated and location added.");
	}
	else {
		dispenseLocation = -1;
		location = "";
		logger->Log(LogLevel::ERROR, "Dispensor ID does not match left or right.");
	}
}

void DispensingHead::SetIngredient(Ingredient* ingredient)
{
	active_ing = ingredient;
	logger->Log(LogLevel::INFO, (location + " Dispensor new ingredient is : " + active_ing->GetName()));
}

void DispensingHead::SetAmountToDispense(double amount_to_dispense)
{
	amount = amount_to_dispense;
	logger->Log(LogLevel::INFO, (location + " Dispensor new ingredient amount is : " + std::to_string(amount)));
}

int DispensingHead::GetLocation()
{
	return dispenseLocation;
}

void DispensingHead::Dispense(Conveyor* conv)
{
	conv->DispenseSauceOn(dispenseLocation, active_ing);
	logger->Log(LogLevel::DEBUG, (location + " Dispensor dispensing " + active_ing->GetName()));

	SimulationExecutive::ScheduleEventIn(taskrate, new DispenseEA(this, conv));
}

// ==== CAROUSEL ====

Carousel::Carousel(int numHeads, Time rate, Time dispensor_taskrate)
{
	positionOffset = 0;
	taskrate = rate;

	dispensingHeads = new DispensingHead*[2];
	dispensingHeads[0] = new DispensingHead(0, this, dispensor_taskrate); // left dispensor
	dispensingHeads[0]->SetAmountToDispense(0.2);

	dispensingHeads[1] = new DispensingHead(1, this, dispensor_taskrate); // right dispensor
	dispensingHeads[1]->SetAmountToDispense(0.2);

	ingredientCartridges = new Ingredient[10]; // specifically 10 cartridges
	ingredientCartridges[0] = Ingredient("Ranch", 10);
	ingredientCartridges[1] = Ingredient("BBQ", 10);
	ingredientCartridges[2] = Ingredient("Mayo", 10);
	ingredientCartridges[3] = Ingredient("Hot Sauce", 10);
	ingredientCartridges[4] = Ingredient("Mustard", 10);
	ingredientCartridges[5] = Ingredient("Teriyaki", 10);
	ingredientCartridges[6] = Ingredient("Caeser", 10);
	ingredientCartridges[7] = Ingredient("Buffalo", 10);
	ingredientCartridges[8] = Ingredient("Chipotle", 10);
	ingredientCartridges[9] = Ingredient("Vinaigrette", 10);
	
	sauceLevels = new double[10];
	for (int i = 0; i < 10; i++) {
		sauceLevels[i] = ingredientCartridges[i].GetLevel();
	}

	UpdateDispensorLocations(positionOffset);

	SimulationExecutive::ScheduleEventIn(taskrate, new AdvanceEA(this));
}

void Carousel::Advance()
{
	positionOffset = (positionOffset + 1) % 10;
	UpdateDispensorLocations(positionOffset);

	SimulationExecutive::ScheduleEventIn(taskrate, new AdvanceEA(this));
}

double Carousel::GetSauceLevel(int index)
{
	return sauceLevels[index];
}

double* Carousel::GetSauceLevels()
{
	return sauceLevels;
}

void Carousel::UpdateDispensorLocations(int offset)
{
	dispensorIngredients[0] = ingredientCartridges[(dispensingHeads[0]->GetLocation() + offset) % 10];
	dispensorIngredients[1] = ingredientCartridges[(dispensingHeads[1]->GetLocation() + offset) % 10];
}

// ==== CONVEYOR ====

Conveyor::Conveyor(int size, Time rate)
{
	taskrate = rate;
	bowls = new Bowl* [size] {};
	slots = size;

	SimulationExecutive::ScheduleEventIn(taskrate, new AdvanceEA(this));
}

void Conveyor::SetQueue(std::vector<Bowl*> orders)
{
	queue = orders;
}

void Conveyor::AddOrdersToQueue(std::vector<Bowl*> orders)
{
	for (auto& order : orders) {
		queue.push_back(order);
	}
}

void Conveyor::Advance()
{
	delete bowls[slots - 1];
	bowls[slots - 1] = nullptr;

	for (int i = (slots - 1); i > 0; i--) {
		bowls[i] = bowls[i - 1];
	}

	bowls[0] = queue.front(); // put new bowl on conveyor
	queue.erase(queue.begin()); // remove it from the queue

	SimulationExecutive::ScheduleEventIn(taskrate, new AdvanceEA(this));
}

void Conveyor::DispenseSauceOn(int index, Ingredient* ingredient)
{
	bowls[index]->FulfillIngredient(*ingredient);
}

int Conveyor::GetBowlWeight(int index)
{
	return bowls[index]->GetWeight();
}

std::vector<int> Conveyor::GetBowlWeights()
{
	std::vector<int> weights;

	for (int i = 0; i < slots; i++) {
		weights.push_back(bowls[i]->GetWeight());
	}

	return weights;
}