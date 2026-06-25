#pragma once
#include <vector>

#include "LoggerLib.h"
#include "Ingredient.h"
#include "Distribution.h"

class Bowl
{
public:
	Bowl(std::vector<Ingredient> required_ingredients, std::vector<int> amounts);
	~Bowl();

	void SetArrivalTime(double time);
	bool FulfillIngredient(Ingredient ingredient);
	bool IsComplete();
	int GetWeight();
	int GetId();

	static void GetStatistics(double& avg_wait_time, double& total_wait_time, int& total_bowls_processed);
	static void SetWeightDistribution(float min, float avg, float max);
private:
	std::vector<Ingredient> required_ingredients;
	std::vector<Ingredient> fulfilled_ingredients;
	std::vector<int> req_ingredient_amounts;
	bool complete;
	int weight;
	int id;

	static Triangular* bowl_weight_distribution;
	static Logger* logger;
	static Logger* results_logger;
	static int bowl_id;
	
	// statistics
	double arrival_time;
	double wait_time;
	static double total_wait_time;
	static int total_bowls_processed;
};

