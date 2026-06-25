#pragma once
#include <vector>

#include "LoggerLib.h"
#include "Ingredient.h"
#include "Distribution.h"
#include "SimulationExecutive.h"

class Bowl
{
public:
	Bowl(std::vector<Ingredient> required_ingredients, std::vector<double> amounts);
	~Bowl();

	void SetArrivalTime(double time);
	bool FulfillIngredient(Ingredient ingredient);
	bool IsComplete();
	double GetWeight();
	int GetId();

	// helpers for making the log look nice
	void WriteExpectationHeader();
	void LogExpectedResult();
	void WriteExpectationFooter();

	static void GetStatistics(Time& avg_wait_time, Time& total_wait_time, int& total_bowls_processed);
	static void SetWeightDistribution(double min, double avg, double max);
private:
	std::vector<Ingredient> required_ingredients;
	std::vector<Ingredient> fulfilled_ingredients;
	std::vector<double> req_ingredient_amounts;
	bool complete;
	double weight;
	int id;

	static Triangular* bowl_weight_distribution;
	static Logger* logger;
	static Logger* results_logger;
	static int bowl_id;
	
	// statistics
	Time arrival_time;
	Time wait_time;
	static Time total_wait_time;
	static int total_bowls_processed;
};

