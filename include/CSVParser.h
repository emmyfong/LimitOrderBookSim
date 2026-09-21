#pragma once

#include "OrderBook.h"
#include <vector>
#include <string>

class CSVParser {
    public:
    //read the csv file and return a list of Order objects
    static std::vector<Order> parse(const std::string&filename);
};