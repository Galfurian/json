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
    const char example[] = "{ /* Comment */\n"
                           "   // This is a comment.\n"
                           "   a: 1, /* Comment */\n"
                           "   // This is another comment.\n"
                           "   /* Comment */ 'b': 2 /* Comment */, /* Comment */ // This is an inline comment.\n"
                           "   // This is the final comment.\n"
                           "/* Comment */ }\n";
    // Parse the json string.
    json::jnode_t root   = json::parser::parse(example);
    // Check correctness.
    if (root.to_string(false) != "{'a': 1,'b': 2}") {
        std::cerr << "Different!\n"
                  << "{'a': 1,'b': 2}"
                  << "\n"
                  << root.to_string(false) << "\n";
        return false;
    }
    return true;
}

int test_comments_in_array(void)
{
    const char example[] = "{\n"
                           "   'a': 1,\n"
                           "    b : 'Hello there!',\n"
                           "   'c': [\n"
                           "       1,// This is an inline comment.\n"
                           "       2, /* Comment */\n"
                           "       3,\n"
                           "   ]\n"
                           "}\n";
    // Parse the json string.
    json::jnode_t root   = json::parser::parse(example);
    // Check correctness.
    if (root.to_string(false) != "{'a': 1,'b': 'Hello there!','c': [1, 2, 3]}") {
        std::cerr << "Different!\n"
                  << "{'a': 1,'b': 'Hello there!','c': [1, 2, 3]}"
                  << "\n"
                  << root.to_string(false) << "\n";
        return false;
    }
    return true;
}

int test_comments_in_inline_json(void)
{
    {
        const char example[] = "{a: 1, b: 'Hello there!', c: [1, 2, 3]}";
        // Parse the json string.
        json::jnode_t root   = json::parser::parse(example);
        // Check correctness.
        if (root.to_string(false) != "{'a': 1,'b': 'Hello there!','c': [1, 2, 3]}") {
            std::cerr << "Different!\n"
                      << "{'a': 1,'b': 'Hello there!','c': [1, 2, 3]}"
                      << "\n"
                      << root.to_string(false) << "\n";
            return false;
        }
    }
    {
        const char example[] = "{a: 1, b: 'Hello there!', c: /* TEST */ [1, 2, 3]}";
        // Parse the json string.
        json::jnode_t root   = json::parser::parse(example);
        // Check correctness.
        if (root.to_string(false) != "{'a': 1,'b': 'Hello there!','c': [1, 2, 3]}") {
            std::cerr << "Different!\n"
                      << "{'a': 1,'b': 'Hello there!','c': [1, 2, 3]}"
                      << "\n"
                      << root.to_string(false) << "\n";
            return false;
        }
    }
    return true;
}

int test_line_break(void)
{
    const char example[] = "{\n"
                           "   'a': 1,\n"
                           "    b : 2,\n"
                           "   'c': 'Lorem ipsum dolor sit amet, \
consectetur adipiscing elit.'\n"
                           "}\n";
    // Parse the json string.
    json::jnode_t root   = json::parser::parse(example);
    // Check correctness.
    if (root.to_string(false) != "{'a': 1,'b': 2,'c': 'Lorem ipsum dolor sit amet, consectetur adipiscing elit.'}") {
        std::cerr << "Different!\n"
                  << "{'a': 1,'b': 2,'c': 'Lorem ipsum dolor sit amet, consectetur adipiscing elit.'}"
                  << "\n"
                  << root.to_string(false) << "\n";
        return false;
    }
    return true;
}

int main(int, char *[])
{
    if (!test_comments_in_ojbect()) {
        return 1;
    }
    if (!test_comments_in_array()) {
        return 1;
    }
    if (!test_comments_in_inline_json()) {
        return 1;
    }
    if (!test_line_break()) {
        return 1;
    }
    return 0;
}
