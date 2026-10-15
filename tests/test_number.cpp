/// @file test_number.cpp
/// @author Enrico Fraccaroli (enry.frak@gmail.com)
/// @brief Tests the JSON5 forms of numbers and how they are read back.
///
/// @copyright (c) 2024 This file is distributed under the MIT License.
/// See LICENSE.md for details.
///

#include <cmath>
#include <iostream>
#include <json/json.hpp>
#include <string>

/// @brief Parses the number and checks its canonical string form.
static bool expect_canonical(const std::string &input, const std::string &expected)
{
    json::jnode_t root = json::parser::parse(input);
    if (root.get_type() != json::JTYPE_NUMBER || root.get_value() != expected) {
        std::cerr << "`" << input << "` became `" << root.get_value() << "`, expected `" << expected << "`.\n";
        return false;
    }
    return true;
}

static bool test_canonical_forms()
{
    return expect_canonical("+1", "1") && expect_canonical(".5", "0.5") && expect_canonical("-.5", "-0.5") &&
           expect_canonical("5.", "5") && expect_canonical("1.e5", "1e5") && expect_canonical("0x10", "16") &&
           expect_canonical("-0xFF", "-255") && expect_canonical("0XaB", "171") &&
           expect_canonical("0xFFFFFFFFFFFFFFFF", "18446744073709551615") && expect_canonical("1E+5", "1e+5") &&
           expect_canonical("-1.5e-3", "-1.5e-3") && expect_canonical("-Infinity", "-Infinity") &&
           expect_canonical("+Infinity", "Infinity") && expect_canonical("NaN", "NaN");
}

static bool test_read_back()
{
    json::jnode_t root = json::parser::parse("[0x10, .5, Infinity, -Infinity, NaN, 1e3]");
    if (root[0].as_number<int>() != 16 || root[5].as_number<int>() != 1000) {
        return false;
    }
    if (std::fabs(root[1].as_number<double>() - 0.5) > 1e-12) {
        return false;
    }
    if (!std::isinf(root[2].as_number<double>()) || root[2].as_number<double>() < 0.0) {
        return false;
    }
    if (!std::isinf(root[3].as_number<double>()) || root[3].as_number<double>() > 0.0) {
        return false;
    }
    if (!std::isnan(root[4].as_number<double>())) {
        return false;
    }
    // Types that cannot hold the special values.
    if (root[2].as_number<int>() != 0) {
        return false;
    }
    json::config::strict_type_check = true;
    bool threw                      = false;
    try {
        (void)root[2].as_number<int>();
    } catch (const json::parser_error &) {
        threw = true;
    }
    json::config::strict_type_check = false;
    return threw;
}

static bool test_round_trip()
{
    json::jnode_t root     = json::parser::parse("{a: [0x10, .5, Infinity, -Infinity, NaN]}");
    json::jnode_t reparsed = json::parser::parse(root.to_string(false));
    return reparsed.to_string(false) == root.to_string(false) && reparsed["a"][0].as_number<int>() == 16;
}

int main(int, char *[])
{
    if (!test_canonical_forms()) {
        return 1;
    }
    if (!test_read_back()) {
        return 1;
    }
    if (!test_round_trip()) {
        return 1;
    }
    return 0;
}
