#include "SauceDispensorRobot.h"

Logger* DispensingHead::logger = Logger::GetInstance("logs", "dispensing_head_log.txt");
Logger* Carousel::logger = Logger::GetInstance("logs", "carousel_log.txt");
Logger* Conveyor::logger = Logger::GetInstance("logs", "conveyor_log.txt");

// ==== EVENT ACTIONS ====

// ==== DISPENSING HEAD ====
class DispensingHead::DispenseEA : public EventAction
{
public:
	DispenseEA(DispensingHead* d)
		: dispensor(d)
	{}

	void Execute() {
		dispensor->Dispense();
	}

private:
	DispensingHead* dispensor;
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

SauceDispensorRobot::SauceDispensorRobot(std::vector<Bowl*> orders, std::vector<Ingredient*> ingredients, Time carousel_taskrate, Time conveyor_taskrate, Time dispensor_taskrate)
{
	conveyor = new Conveyor(6, conveyor_taskrate);
	conveyor->SetQueue(orders);

	carousel = new Carousel(2, carousel_taskrate, dispensor_taskrate, ingredients, conveyor);
}

void SauceDispensorRobot::AddOrders(std::vector<Bowl*> orders)
{
	conveyor->AddOrdersToQueue(orders);
}


// ================ SIMULATED HARDWARE ===============
// ==== DISPENSING HEAD ====

DispensingHead::DispensingHead(int head_id, Carousel* c, Conveyor* conv, Time rate)
{
	carousel = c;
	conveyor = conv;
	active_ing = nullptr;
	amount = -1.0;
	taskrate = rate;
	id = head_id;

	if (id == 0) {
		dispenseLocation = 1; // slot 1 on the conveyor based on the image
		location = "Left";
		DispensingHead::logger->Log(LogLevel::INFO, "Left Dispensor instantiated and location added.");
	}
	else if (id == 1) {
		dispenseLocation = 4; // slot 4 on the conveyor based on the image
		location = "Right";
		DispensingHead::logger->Log(LogLevel::INFO, "Right Dispensor instantiated and location added.");
	}
	else {
		dispenseLocation = -1;
		location = "";
		DispensingHead::logger->Log(LogLevel::ERROR, "Dispensor ID does not match left or right.");
	}

	ScheduleEventIn(taskrate, new DispenseEA(this));
}

void DispensingHead::SetIngredient(Ingredient* ingredient)
{
	active_ing = ingredient;
	//DispensingHead::logger->Log(LogLevel::INFO, (location + " Dispensor new ingredient is : " + active_ing->GetName()));
}

void DispensingHead::SetAmountToDispense(double amount_to_dispense)
{
	amount = amount_to_dispense;
	//DispensingHead::logger->Log(LogLevel::INFO, (location + " Dispensor new ingredient amount is : " + std::to_string(amount)));
}

int DispensingHead::GetLocation()
{
	return dispenseLocation;
}

void DispensingHead::Dispense()
{
	if (conveyor->DispenseSauceOn(dispenseLocation, active_ing)) {
		carousel->ReduceSauceLevel(id, amount);
	}
	DispensingHead::logger->Log(LogLevel::DEBUG, (location + " Dispensor dispensing " + active_ing->GetName()));

	ScheduleEventIn(taskrate, new DispenseEA(this));
}

// ==== CAROUSEL ====

Carousel::Carousel(int numHeads, Time rate, Time dispensor_taskrate, std::vector<Ingredient*> ingredients, Conveyor* conv)
{
	positionOffset = 0;
	taskrate = rate;

	dispensingHeads.push_back(new DispensingHead(0, this, conv, dispensor_taskrate)); // left dispensor
	dispensingHeads[0]->SetAmountToDispense(0.2);

	dispensingHeads.push_back(new DispensingHead(1, this, conv, dispensor_taskrate)); // right dispensor
	dispensingHeads[1]->SetAmountToDispense(0.2);

	ingredientCartridges = ingredients;
	for (int i = 0; i < ingredients.size(); i++) {
		sauceLevels.push_back(ingredients[i]->GetLevel());
	}

	UpdateDispensorLocations(positionOffset);

	ScheduleEventIn(taskrate, new AdvanceEA(this));
}

void Carousel::ReduceSauceLevel(int head, double amount)
{
	if (head == 0 || head == 1) {
		bool sauceHasCapacity = dispensorIngredients[head]->ReduceLevel(0.2);
		if (sauceHasCapacity) {
			Carousel::logger->Log(LogLevel::INFO, "Reduced level of " + dispensorIngredients[head]->GetName() + " from " + std::to_string(dispensorIngredients[head]->GetLevel() + 0.2) + " to " + std::to_string(dispensorIngredients[head]->GetLevel()));
		}
		else {
			Carousel::logger->Log(LogLevel::INFO, "Failed to reduced level of " + dispensorIngredients[head]->GetName() + " due to empty container.");
		}
	}
}

void Carousel::SetIngredientCartridges(std::vector<Ingredient*> ingredients)
{
	ingredientCartridges = ingredients;
	sauceLevels.clear();
	for (int i = 0; i < ingredients.size(); i++) {
		sauceLevels.push_back(ingredients[i]->GetLevel());
	}
}

void Carousel::Advance()
{
	positionOffset = (positionOffset + 1) % 10;
	UpdateDispensorLocations(positionOffset);

	ScheduleEventIn(taskrate, new AdvanceEA(this));
}

double Carousel::GetSauceLevel(int index)
{
	return sauceLevels[index];
}

std::vector<double> Carousel::GetSauceLevels()
{
	return sauceLevels;
}

void Carousel::UpdateDispensorLocations(int offset)
{
	int leftCartridge = (8 - offset);
	int rightCartridge = (2 - offset);

	if (leftCartridge < 0) { leftCartridge += 10; }
	if (rightCartridge < 0) { rightCartridge += 10; }

	if (dispensorIngredients.size() == 0) {
		dispensorIngredients.push_back(ingredientCartridges[leftCartridge]);
		dispensorIngredients.push_back(ingredientCartridges[rightCartridge]);
	}
	else {
		dispensorIngredients[0] = ingredientCartridges[leftCartridge];
		dispensorIngredients[1] = ingredientCartridges[rightCartridge];
	}

	dispensingHeads[0]->SetIngredient(dispensorIngredients[0]);
	dispensingHeads[1]->SetIngredient(dispensorIngredients[1]);
}

// ==== CONVEYOR ====

Conveyor::Conveyor(int size, Time rate)
{
	taskrate = rate;
	for (int i = 0; i < size; i++) {
		bowls.push_back(nullptr);
	}

	slots = size;

	ScheduleEventIn(taskrate, new AdvanceEA(this));
}

void Conveyor::SetQueue(std::vector<Bowl*> orders)
{
	orders[0]->WriteExpectationHeader();
	for (auto& order : orders) {
		order->LogExpectedResult();
		queue.push_back(std::make_unique<Bowl>(*order));
	}
	orders[0]->WriteExpectationFooter();
}

void Conveyor::AddOrdersToQueue(std::vector<Bowl*> orders)
{
	for (auto& order : orders) {
		queue.push_back(std::make_unique<Bowl>(*order));
	}
}

void Conveyor::Advance()
{
	for (int i = (slots - 1); i > 0; i--) {
		// swap the pointers between this bowl and the previous
		// with the current bowl going out of scope and forcing the destructor to capture statistics
		// example at https://en.cppreference.com/cpp/utility/move

		bowls[i] = std::move(bowls[i - 1]); 
	}

	// only add a new bowl if there is a new bowl to put in
	if (queue.size() > 0) {
		bowls[0] = std::move(queue.front()); // put new bowl on conveyor, same memory management deal as above
		bowls[0]->SetArrivalTime(GetSimulationTime());
		queue.erase(queue.begin()); // remove it from the queue
	}

	ScheduleEventIn(taskrate, new AdvanceEA(this));
}

bool Conveyor::DispenseSauceOn(int index, Ingredient* ingredient)
{
	if (bowls[index] != nullptr) {
		bool fulfilled = bowls[index]->FulfillIngredient(*ingredient);
		if (fulfilled) {
			Conveyor::logger->Log(LogLevel::INFO, "Bowl " + std::to_string(bowls[index]->GetId()) + " " + ingredient->GetName() + " fulfilled.");
			return true;
		}
		else {
			Conveyor::logger->Log(LogLevel::DEBUG, "Attempted to dispense " + ingredient->GetName() + " on Bowl " + std::to_string(bowls[index]->GetId()) + " that did not need it.");
			return false;
		}
	}

	return false;
}

double Conveyor::GetBowlWeight(int index)
{
	return bowls[index]->GetWeight();
}

std::vector<double> Conveyor::GetBowlWeights()
{
	std::vector<double> weights;

	for (int i = 0; i < slots; i++) {
		weights.push_back(bowls[i]->GetWeight());
	}

	return weights;
}