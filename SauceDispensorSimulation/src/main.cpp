#include "SimulationExecutive.h"
#include "SauceDispensorRobot.h"

int main() {

	SimulationExecutive::InitializeSimulation();

	//SauceDispensorRobot* robot = new SauceDispensorRobot();
	//robot->DispenseSauce();

	SimulationExecutive::RunSimulation();
}