#include "Simulator.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    std::cout << "=========================================\n";
    std::cout << "   HIGH-FREQUENCY LOB SIMULATOR (v1.0)   \n";
    std::cout << "=========================================\n";

    //defaults to the sample data file; pass a path to use a different one
    std::string dataFile = (argc > 1) ? argv[1] : "data/orders.csv";

    Simulator engine(dataFile);
    engine.run();

    return 0;
}
