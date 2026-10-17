/// @file test_options.cpp
/// @author Enrico Fraccaroli (enry.frak@gmail.com)
/// @brief Tests the per-call parse options and the file parsing.
///
/// @copyright (c) 2024 This file is distributed under the MIT License.
/// See LICENSE.md for details.
///

#include <cstdio>
#include <fstream>
#include <iostream>
#include <json/json.hpp>
#include <string>

template <typename Function>
static bool throws_parser_error(Function function)
{
    try {
        function();
    } catch (const json::parser_error &) {
        return true;
    }
    return false;
}

static bool test_max_depth_option()
{
    json::parse_options options;
    options.max_depth = 3;
    // The global limit is not touched.
    if (json::config::max_depth != json::parse_options().max_depth) {
        return false;
    }
    json::parser::parse("[[[1]]]", options);
    return throws_parser_error([&]() { json::parser::parse("[[[[1]]]]", options); }) &&
           // Without options the global default applies.
           !throws_parser_error([]() { json::parser::parse("[[[[1]]]]"); });
}

static bool test_max_input_size_option()
{
    json::parse_options options;
    options.max_input_size = 8;
    json::parser::parse("[1,2,3]", options);
    return throws_parser_error([&]() { json::parser::parse("[1,2,3,4,5]", options); });
}

static bool test_global_defaults()
{
    json::config::max_input_size = 4;
    bool rejected                = throws_parser_error([]() { json::parser::parse("[1,2,3]"); });
    json::config::max_input_size = 0;
    return rejected && !throws_parser_error([]() { json::parser::parse("[1,2,3]"); });
}

static bool test_parse_file()
{
    const std::string filename = "json_test_options_tmp.json";
    {
        std::ofstream out(filename.c_str());
        out << "{ a: [1, 2, 3] }";
    }
    bool ok            = true;
    json::jnode_t root = json::parser::parse_file(filename);
    ok                 = ok && (root["a"].size() == 3);
    json::parse_options options;
    options.max_input_size = 4;
    ok                     = ok && throws_parser_error([&]() { json::parser::parse_file(filename, options); });
    std::remove(filename.c_str());
    // A missing file is an error, not a null node.
    ok = ok && throws_parser_error([&]() { json::parser::parse_file(filename); });
    // Unless the caller asks for the old behavior.
    json::parse_options lenient;
    lenient.missing_file_as_null = true;
    ok                           = ok && json::parser::parse_file(filename, lenient).is_null();
    // The option does not hide invalid content.
    {
        std::ofstream out(filename.c_str());
        out << "{ a: ";
    }
    ok = ok && throws_parser_error([&]() { json::parser::parse_file(filename, lenient); });
    std::remove(filename.c_str());
    return ok;
}

int main(int, char *[])
{
    if (!test_max_depth_option()) {
        std::cerr << "test_max_depth_option failed\n";
        return 1;
    }
    if (!test_max_input_size_option()) {
        std::cerr << "test_max_input_size_option failed\n";
        return 1;
    }
    if (!test_global_defaults()) {
        std::cerr << "test_global_defaults failed\n";
        return 1;
    }
    if (!test_parse_file()) {
        std::cerr << "test_parse_file failed\n";
        return 1;
    }
    return 0;
}
