/// @file test_error.cpp
/// @author Enrico Fraccaroli (enry.frak@gmail.com)
/// @brief Tests the error reporting capability of the json library.
///
/// @copyright (c) 2024 This file is distributed under the MIT License.
/// See LICENSE.md for details.
///

#include <iostream>
#include <json/json.hpp>
#include <limits>
#include <sstream>

bool test_error_format()
{
    const char input[] = "{\n"               // 1
                         "   'v0': 1,\n"     // 2
                         "   'v1': false,\n" // 3
                         "   'v2': 1.5,\n"   // 4
                         "\n";               // 5
    try {
        json::jnode_t root = json::parser::parse(input);
    } catch (json::parser_error &e) {
        std::cout << "L" << e.line << " : " << e.what() << "\n";
        return true;
    }
    std::cout << "We did not detect an error in the format.\n";
    return false;
}

bool test_error_type()
{
    const char input[] = "{\n"              // 1
                         "   'v0': 1,\n"    // 2
                         "   'v1': true,\n" // 3
                         "   'v2': 1.5,\n"  // 4
                         "}\n";             // 5
    try {
        json::jnode_t root = json::parser::parse(input);
        float v1;
        root["v1"] >> v1;
    } catch (json::type_error &e) {
        std::cout << "L" << e.line << " : " << e.found << " vs " << e.expected << " : " << e.what() << "\n";
        return true;
    }
    std::cout << "We did not detect a type error.\n";
    return false;
}

bool test_error_out_of_bound()
{
    const char input[] = "{\n"               // 1
                         "   'v0': 1,\n"     // 2
                         "   'v1': false,\n" // 3
                         "   'v2': 1.5,\n"   // 4
                         "}\n";              // 5
    try {
        json::jnode_t root = json::parser::parse(input);
        float v1;
        root[0] >> v1;
        root[5] >> v1;
    } catch (json::range_error &e) {
        std::cout << "L" << e.line << " : " << e.index << " >= " << e.size << " : " << e.what() << "\n";
        return true;
    }
    std::cout << "We did not detect an out-of-bound error.\n";
    return false;
}

bool test_error_parser()
{
    const char input[] = "{\n"             // 1
                         "   'v0': 1,\n"   // 2
                         "   'v1': ,\n"    // 3
                         "   'v2': 1.5,\n" // 4
                         "}\n";            // 5
    try {
        json::jnode_t root = json::parser::parse(input);
        float v1;
        root[0] >> v1;
        std::cout << root << "\n";
    } catch (json::parser_error &e) {
        std::cout << "L" << e.line << " : " << e.what() << "\n";
        return true;
    }
    std::cout << "We did not detect a parsing error.\n";
    return false;
}

bool test_error_no_entry()
{
    const char input[] = "{\n"               // 1
                         "   'v0': 1,\n"     // 2
                         "   'v1': false,\n" // 3
                         "   'v2': 1.5,\n"   // 4
                         "}\n";              // 5
    try {
        json::jnode_t root = json::parser::parse(input);
        float v3;
        root["v3"] >> v3;
    } catch (json::parser_error &e) {
        std::cout << "L" << e.line << " : " << e.what() << "\n";
        return true;
    }
    std::cout << "We did not detect an error with a missing entry.\n";
    return false;
}

bool test_error_number_overflow()
{
    json::jnode_t root = json::parser::parse("[300, 99999999999, 1e400, 12]");
    // These fit.
    if (root[0].as_number<int>() != 300 || root[3].as_number<int>() != 12) {
        std::cout << "Valid numbers were not read correctly.\n";
        return false;
    }
    // These do not.
    try {
        (void)root[1].as_number<int>();
        std::cout << "We did not detect an int overflow.\n";
        return false;
    } catch (json::parser_error &e) {
        std::cout << "L" << e.line << " : " << e.what() << "\n";
    }
    try {
        (void)root[2].as_number<double>();
        std::cout << "We did not detect a double overflow.\n";
        return false;
    } catch (json::parser_error &e) {
        std::cout << "L" << e.line << " : " << e.what() << "\n";
    }
    // Without the strict check we get the closest value.
    json::config::strict_type_check = false;
    bool saturated                  = root[1].as_number<int>() == std::numeric_limits<int>::max();
    json::config::strict_type_check = true;
    if (!saturated) {
        std::cout << "The overflowing value was not saturated.\n";
    }
    return saturated;
}

int main(int, char *[])
{
    json::config::strict_type_check      = true;
    json::config::strict_existance_check = true;

    if (!test_error_format()) {
        return 1;
    }
    if (!test_error_type()) {
        return 1;
    }
    if (!test_error_out_of_bound()) {
        return 1;
    }
    if (!test_error_parser()) {
        return 1;
    }
    if (!test_error_no_entry()) {
        return 1;
    }
    if (!test_error_number_overflow()) {
        return 1;
    }
    return 0;
}
