# SauceDispensorSimulation

## Overview

SauceDispensorSimulation is a C++ discrete-event simulation of a sauce dispensing robot. It models an automated system with a rotating carousel, conveyor belt, and two dispensing heads that attempt to fulfill randomized bowl orders from a queue.

The project is designed to demonstrate simulation architecture, event-driven scheduling, and hardware modeling in a lightweight console application.

## Links

- Source code: https://github.com/npick001/SauceDispensorSimulation
- Portfolio: https://portfolio.nicholaspickering.dev/
- Custom logger library: https://github.com/npick001/LoggerLib

## Features

- Discrete-event simulation engine driving time and scheduled events
- Carousel with two dispensing heads and rotating ingredient cartridges
- Conveyor belt queue managing bowl orders
- Random bowl order generation using ingredient sets and amounts
- Custom logging integration via the `LoggerLib` library
- Output logs stored in the `logs/` directory for analysis

## Project Structure

- `SauceDispensorSimulation.slnx` — Visual Studio solution file
- `SauceDispensorSimulation/` — main Visual Studio project folder
  - `inc/` — public headers for the simulation components
  - `src/` — source implementation files
  - `lib/` — library dependencies for `LoggerLib`
  - `logs/` — runtime log output
- `Assumptions.txt` — documented project assumptions for hardware and simulation behavior

## Build and Run

1. Open `SauceDispensorSimulation.slnx` in Visual Studio.
2. Ensure the project is configured to use the `x64` platform and the desired configuration (`Debug` or `Release`).
3. Build the solution.
4. Run the resulting executable.

### Requirements

- Microsoft Visual Studio with C++ support
- C++20 language standard support
- `LoggerLib` dependency available via the included `lib/` and `inc/` paths or from https://github.com/npick001/LoggerLib

## Logging

The simulation uses the custom `LoggerLib` library for structured logging. Log output files are created in the `SauceDispensorSimulation/logs/` folder, including:

- `bowl_log.txt`
- `carousel_log.txt`
- `conveyor_log.txt`
- `dispensing_head_log.txt`
- `results_log.txt`

## Assumptions

The simulation is built around a specific set of hardware and model assumptions, captured in `Assumptions.txt` and summarized here:

### Hardware assumptions

- Conveyor and carousel move at consistent, fixed speeds.
- Dispensing heads dispense sauce instantly when triggered.
- Dispensing heads know which ingredient they are dispensing and the amount to dispense.
- Each dispense operation uses an exact, fixed amount of sauce.
- There is no simulated hardware failure behavior.

### Model implementation assumptions

- The system is modeled using a discrete-event simulation executive.
- The carousel is represented by rotating ingredient pointers rather than moving physical cartridges.
- Two dispensing heads are locked to carousel positions and update their active ingredient as the carousel rotates.
- The conveyor manages the order queue and advances bowls through slots on a fixed timer.
- Ingredients are limited to a fixed list of sauce types.
- All ingredient cartridges are assumed to have equal capacity.
- Bowls may require zero to two distinct, non-stacking ingredients.
- Dispensing heads may attempt to apply sauce that a particular bowl does not need.

### Simulation defaults

The sample configuration uses example timings and values that are not calibrated to real hardware:

- `carousel_taskrate = 50 ms`
- `conveyor_taskrate = 500 ms`
- `dispensor_taskrate = 50 ms`
- `bowl_weight_distribution = Triangular [1.0, 2.0, 3.0]`
- `Ingredient amount_to_dispense = 0.2`
- `Ingredient capacity = 10`

## Notes

- The simulation does not currently model vision-based correction or hardware failure modes.
- The random order generator uses a discrete set of 10 predefined ingredients.
- The result logs are useful for reviewing how bowls moved through the conveyor and which ingredients were dispensed.

## Contact

For more projects, visit my portfolio at https://portfolio.nicholaspickering.dev/.
