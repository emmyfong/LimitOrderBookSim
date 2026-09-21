#include "CSVParser.h"
#include <fstream>
#include <sstream>
#include <iostream>

std::vector<Order> CSVParser::parse(const std::string& filename) {
    std::vector<Order> orders;

    //std::ifstream opens file for reading
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "ERROR: Could not open file " << filename << "\n";
        return orders;
    }

    std::string line;

    //loop to get every remaining line in the file
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string token;
        Order order;

        //get id
        std::getline(ss, token, ',');
        //convert string to unsigned long long
        order.orderId = std::stoull(token);

        //side
        std::getline(ss, token, ',');
        order.side = (token == "BUY") ? OrderSide::BUY : OrderSide::SELL;

        //type
        std::getline(ss, token, ',');
        order.type = (token == "LIMIT") ? OrderType::LIMIT : OrderType::MARKET;

        //price
        std::getline(ss, token, ',');
        order.price = std::stoull(token);

        //quantity
        std::getline(ss, token, ',');
        order.quantity = std::stoul(token); //string to unsigned long

        //timestamp
        std::getline(ss, token, ',');
        order.timestamp = std::stoull(token);

        orders.push_back(order);
    }

    return orders;
}