#include "Bowl.h"

Triangular* Bowl::bowl_weight_distribution = nullptr;
Logger* Bowl::logger = Logger::GetInstance("logs", "bowl_log.txt", 31);
Logger* Bowl::results_logger = Logger::GetInstance("logs", "results_log.txt", 31);
int Bowl::bowl_id = 0;
double Bowl::total_wait_time = 0.0;
int Bowl::total_bowls_processed = 0;

Bowl::Bowl(std::vector<Ingredient> requirements, std::vector<int> amounts)
{
	required_ingredients = requirements;
	req_ingredient_amounts = amounts;
	complete = false;

	id = bowl_id;
	bowl_id++;
	wait_time = 0.0;
	arrival_time = -1.0;

	// randomly generate the current bowl weight on instantiation
	if (bowl_weight_distribution == nullptr) {
		SetWeightDistribution(1.0, 2.0, 3.0); // default distribution if not set
	}

	weight = bowl_weight_distribution->GetRV();	
}

Bowl::~Bowl()
{
	// update statistics 
	wait_time = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count() - arrival_time;
	total_bowls_processed++;
	total_wait_time += wait_time;

	if (required_ingredients.size() > 0) {
		logger->Log(LogLevel::WARNING, "Bowl: Destroying bowl with unfulfilled ingredients.");
		results_logger->Log(LogLevel::WARNING, "Bowl: Didn't fulfill " + std::to_string(required_ingredients.size()) + " ingredients.");
		for (auto& ingredient : required_ingredients) {
			results_logger->Log(LogLevel::WARNING, "Bowl: Unfulfilled Ingredient: " + ingredient.GetName());
		}
	}

	// write to log the number and fulfilled ingredients
	results_logger->Log(LogLevel::INFO, "Bowl: Fulfilled " + std::to_string(fulfilled_ingredients.size()) + "ingredients: ");
	for (auto& ingredient : fulfilled_ingredients) {
		results_logger->Log(LogLevel::INFO, "Bowl: Ingredient: " + ingredient.GetName());
	}
}

void Bowl::GetStatistics(double& avg_wait_time, double& total_wait_time, int& total_bowls_processed)
{
	avg_wait_time = Bowl::total_wait_time / Bowl::total_bowls_processed;
	total_wait_time = Bowl::total_wait_time;
	total_bowls_processed = Bowl::total_bowls_processed;
}

void Bowl::SetWeightDistribution(float min, float avg, float max)
{
	bowl_weight_distribution = new Triangular(min, avg, max);
	logger->Log(LogLevel::INFO, "Bowl: Set weight distribution to min: " + std::to_string(min) + ", avg: " + std::to_string(avg) + ", max: " + std::to_string(max));
}

void Bowl::SetArrivalTime(double time)
{
	arrival_time = time;
}

bool Bowl::FulfillIngredient(Ingredient ingredient)
{
	for (auto it = required_ingredients.begin(); it != required_ingredients.end(); ++it) {

		if (*it == ingredient) {
			fulfilled_ingredients.push_back(ingredient); // keep track of fulfilled ingredients

			std::swap(*it, required_ingredients.back());
			required_ingredients.pop_back();

			logger->Log(LogLevel::INFO, "Bowl: Fulfilled ingredient: " + ingredient.GetName());
			return true;
		}
	}

	logger->Log(LogLevel::WARNING, "Bowl: Attempted to fulfill ingredient not required: " + ingredient.GetName());
	return false;
}

bool Bowl::IsComplete()
{
	return complete;
}

int Bowl::GetWeight()
{
	return weight;
}

int Bowl::GetId()
{
	return id;
}
