/// @file test_struct.cpp
/// @author Enrico Fraccaroli (enry.frak@gmail.com)
/// @brief Tests the ability of the json library to parse structures.
///
/// @copyright (c) 2024 This file is distributed under the MIT License.
/// See LICENSE.md for details.
///

#include <iostream>
#include <json/json.hpp>
#include <sstream>

int test_comments(void)
{
    const char example[] =
        "{\n"
        "   // This is a comment.\n"
        "   'a': 1,\n"
        "   // This is another comment.\n"
        "   'b': 2, // This is an inline comment.\n"
        "   // This is the final comment.\n"
        "}\n";
    // Parse the json string.
    json::jnode_t root = json::parser::parse(example);
    return (root.to_string(false) == "{\"a\": 1,\"b\": 2}");
}

int main(int, char *[])
{
    return !test_comments();
}
