#include "Simulator.h"
#include <iostream>

//Helper vars
uint64_t currentTimestep = 1;
uint64_t getNextTime() { return currentTimestep++; }

int main() {
    std::cout << "=========================================\n";
    std::cout << "   HIGH-FREQUENCY LOB SIMULATOR (v1.0)   \n";
    std::cout << "=========================================\n";

    //instantiate engine with data file
    Simulator engine("data/orders.csv");

    //Run multithreaded simulation
    engine.run();

    return 0;    
}