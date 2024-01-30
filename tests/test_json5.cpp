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

int test_comments_in_ojbect(void)
{
    const char example[] =
        "{ /* Comment */\n"
        "   // This is a comment.\n"
        "   'a': 1, /* Comment */\n"
        "   // This is another comment.\n"
        "   /* Comment */ 'b': 2 /* Comment */, /* Comment */ // This is an inline comment.\n"
        "   // This is the final comment.\n"
        "/* Comment */ }\n";
    // Parse the json string.
    json::jnode_t root = json::parser::parse(example);
    return (root.to_string(false) == "{\"a\": 1,\"b\": 2}");
}

int test_comments_in_array(void)
{
    const char example[] =
        "{\n"
        "   'a': 1,\n"
        "   'b': 2,\n"
        "   'c': {\n"
        "       1,// This is an inline comment.\n"
        "       2, /* Comment */\n"
        "       3,\n"
        "   }\n"
        "}\n";
    // Parse the json string.
    json::jnode_t root = json::parser::parse(example);
    return (root.to_string(false) == "{\"a\": 1,\"b\": 2}");
}

int main(int, char *[])
{
    if(test_comments_in_ojbect())
    {
        return 1;
    }
    if(test_comments_in_array())
    {
        return 1;
    }
    return 0;
}
