#include <iostream>
#include <sstream>

#include <json/json.hpp>

bool test_error_format()
{
    const char input[] =
        "{\n"               // 1
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
    const char input[] =
        "{\n"              // 1
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
    const char input[] =
        "{\n"               // 1
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
    const char input[] =
        "{\n"             // 1
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
    const char input[] =
        "{\n"               // 1
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
    return 0;
}