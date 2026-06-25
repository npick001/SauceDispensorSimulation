#include <unordered_set>
#include "SimulationExecutive.h"
#include "SauceDispensorRobot.h"

std::vector<Bowl*> GenerateSampleOrders(int num_orders, std::vector<Ingredient*> available_ingredients) {
	std::vector<Bowl*> orders;
	Uniform* num_ingredients = new Uniform(0, 3);
	Uniform* ing_sampler = new Uniform(0, 10);

	for (int i = 0; i < num_orders; i++) {
		
		// generate random ingredients for this bowl
		std::unordered_set<int> ing_indeces;
		std::vector<Ingredient> ingredients;
		std::vector<double> amounts;
		int number_of_bowl_ingredients = (int)num_ingredients->GetRV();
		while (ing_indeces.size() < number_of_bowl_ingredients) {
			int index = (int)ing_sampler->GetRV();
			ing_indeces.insert(index);
			ingredients.push_back(*available_ingredients[index]);
			amounts.push_back(0.2);
		}

		// create the bowl
		orders.push_back(new Bowl(ingredients, amounts));
	}

	return orders;
}

int main() {
	int carousel_taskrate = 50; // ms
	int conveyor_taskrate = 500; // ms
	int dispensor_taskrate = 50; // ms

	std::vector<Ingredient*> cartridge_ingredients;
	cartridge_ingredients.push_back(new Ingredient("Ranch", 10));
	cartridge_ingredients.push_back(new Ingredient("BBQ", 10));
	cartridge_ingredients.push_back(new Ingredient("Mayo", 10));
	cartridge_ingredients.push_back(new Ingredient("Hot Sauce", 10));
	cartridge_ingredients.push_back(new Ingredient("Mustard", 10));
	cartridge_ingredients.push_back(new Ingredient("Teriyaki", 10));
	cartridge_ingredients.push_back(new Ingredient("Caeser", 10));
	cartridge_ingredients.push_back(new Ingredient("Buffalo", 10));
	cartridge_ingredients.push_back(new Ingredient("Chipotle", 10));
	cartridge_ingredients.push_back(new Ingredient("Vinaigrette", 10));

	InitializeSimulation();

	std::vector<Bowl*> simulated_orders = GenerateSampleOrders(20, cartridge_ingredients);
	SauceDispensorRobot* robot = new SauceDispensorRobot(simulated_orders, cartridge_ingredients, carousel_taskrate, conveyor_taskrate, dispensor_taskrate);

	RunSimulation(100000);
}