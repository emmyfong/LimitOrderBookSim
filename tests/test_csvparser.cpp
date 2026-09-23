#include "CSVParser.h"
#include <cassert>
#include <cstdio> //for std::remove
#include <fstream>
#include <iostream>

namespace {

//Writes a scratch CSV file with the given content so CSVParser::parse
//(which only takes a filename) can be exercised end-to-end.
void writeCsv(const std::string& path, const std::string& content) {
    std::ofstream out(path);
    out << content;
}

} // namespace

void test_parses_valid_line() {
    const std::string path = "test_scratch_valid.csv";
    writeCsv(path, "1,100,BUY,LIMIT,15000,100,42\n");

    std::vector<Order> orders = CSVParser::parse(path);
    std::remove(path.c_str());

    assert(orders.size() == 1);
    assert(orders[0].orderId == 1);
    assert(orders[0].traderId == 100);
    assert(orders[0].side == OrderSide::BUY);
    assert(orders[0].type == OrderType::LIMIT);
    assert(orders[0].price == 15000);
    assert(orders[0].quantity == 100);
    assert(orders[0].timestamp == 42);

    std::cout << "[PASS] Parses Valid Line Test\n";
}

void test_skips_non_numeric_field_without_crashing() {
    const std::string path = "test_scratch_bad_number.csv";
    writeCsv(path,
        "1,100,BUY,LIMIT,15000,100,1\n"
        "2,100,BUY,LIMIT,notanumber,100,2\n"
        "3,100,BUY,LIMIT,15000,100,3\n");

    std::vector<Order> orders = CSVParser::parse(path);
    std::remove(path.c_str());

    // The malformed middle line should be skipped, not crash the parser -
    // only the two valid lines survive.
    assert(orders.size() == 2);
    assert(orders[0].orderId == 1);
    assert(orders[1].orderId == 3);

    std::cout << "[PASS] Skips Non-Numeric Field Without Crashing Test\n";
}

void test_skips_line_with_missing_fields() {
    const std::string path = "test_scratch_missing_fields.csv";
    writeCsv(path,
        "1,100,BUY,LIMIT,15000,100,1\n"
        "2,100,BUY,LIMIT\n"
        "3,100,SELL,LIMIT,15100,50,3\n");

    std::vector<Order> orders = CSVParser::parse(path);
    std::remove(path.c_str());

    assert(orders.size() == 2);
    assert(orders[0].orderId == 1);
    assert(orders[1].orderId == 3);

    std::cout << "[PASS] Skips Line With Missing Fields Test\n";
}

void test_rejects_invalid_side_instead_of_defaulting() {
    const std::string path = "test_scratch_bad_side.csv";
    writeCsv(path, "1,100,BUYY,LIMIT,15000,100,1\n");

    std::vector<Order> orders = CSVParser::parse(path);
    std::remove(path.c_str());

    // A typo'd side must be rejected, not silently coerced into SELL
    assert(orders.empty());

    std::cout << "[PASS] Rejects Invalid Side Instead Of Defaulting Test\n";
}

void test_rejects_invalid_type_instead_of_defaulting() {
    const std::string path = "test_scratch_bad_type.csv";
    writeCsv(path, "1,100,BUY,LIMITT,15000,100,1\n");

    std::vector<Order> orders = CSVParser::parse(path);
    std::remove(path.c_str());

    // A typo'd type must be rejected, not silently coerced into MARKET
    assert(orders.empty());

    std::cout << "[PASS] Rejects Invalid Type Instead Of Defaulting Test\n";
}

void test_skips_blank_lines() {
    const std::string path = "test_scratch_blank_lines.csv";
    writeCsv(path, "1,100,BUY,LIMIT,15000,100,1\n\n3,100,SELL,LIMIT,15100,50,3\n");

    std::vector<Order> orders = CSVParser::parse(path);
    std::remove(path.c_str());

    assert(orders.size() == 2);

    std::cout << "[PASS] Skips Blank Lines Test\n";
}

void test_missing_file_returns_empty() {
    std::vector<Order> orders = CSVParser::parse("this_file_does_not_exist.csv");
    assert(orders.empty());

    std::cout << "[PASS] Missing File Returns Empty Test\n";
}

int main() {
    std::cout << "Running CSVParser Unit Tests...\n";

    test_parses_valid_line();
    test_skips_non_numeric_field_without_crashing();
    test_skips_line_with_missing_fields();
    test_rejects_invalid_side_instead_of_defaulting();
    test_rejects_invalid_type_instead_of_defaulting();
    test_skips_blank_lines();
    test_missing_file_returns_empty();

    std::cout << "ALL TESTS PASSED SUCCESSFULLY.\n";
    return 0;
}
