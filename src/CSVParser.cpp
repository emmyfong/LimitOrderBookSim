#include "CSVParser.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

namespace {

//Read the next field or throw if the line run out of fields early
std::string nextField(std::stringstream& ss, const char* fieldName) {
    std::string token;
    if (!std::getline(ss, token, ',')) {
        throw std::invalid_argument(std::string("missing field: ") + fieldName);
    }
    return token;
}

//Parses one csv line into an order
Order parseLine(const std::string& line) {
    std::stringstream ss(line);
    Order order{};

    //convert string to unsigned long long
    order.orderId = std::stoull(nextField(ss, "orderId"));
    order.traderId = std::stoull(nextField(ss, "traderId"));

    std::string side = nextField(ss, "side");
    if (side == "BUY") {
        order.side = OrderSide::BUY;
    } else if (side == "SELL") {
        order.side = OrderSide::SELL;
    } else {
        throw std::invalid_argument("invalid side: " + side);
    }

    std::string type = nextField(ss, "type");
    if (type == "LIMIT") {
        order.type = OrderType::LIMIT;
    } else if (type == "MARKET") {
        order.type = OrderType::MARKET;
    } else {
        throw std::invalid_argument("invalid type: " + type);
    }

    order.price = std::stoull(nextField(ss, "price"));
    order.quantity = std::stoul(nextField(ss, "quantity")); //string to unsigned long
    order.timestamp = std::stoull(nextField(ss, "timestamp"));

    return order;
}

} // namespace

std::vector<Order> CSVParser::parse(const std::string& filename) {
    std::vector<Order> orders;

    //std::ifstream opens file for reading
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "ERROR: Could not open file " << filename << "\n";
        return orders;
    }

    std::string line;
    int lineNumber = 0;

    //loop to get every remaining line in the file
    while (std::getline(file, line)) {
        ++lineNumber;

        if (line.empty()) {
            continue;
        }

        try {
            orders.push_back(parseLine(line));
        } catch (const std::exception& e) {
            std::cerr << "WARNING: Skipping malformed line " << lineNumber
                       << " (" << e.what() << "): " << line << "\n";
        }
    }

    return orders;
}
