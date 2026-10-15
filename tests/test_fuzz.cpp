/// @file test_fuzz.cpp
/// @author Enrico Fraccaroli (enry.frak@gmail.com)
/// @brief Feeds randomly mutated documents to the parser.
/// @details The only acceptable outcomes are a parsed tree or a parser_error.
/// Anything else (a crash, another exception, a sanitizer report) is a bug.
/// The generator is seeded, so every run exercises the same inputs.
///
/// @copyright (c) 2024 This file is distributed under the MIT License.
/// See LICENSE.md for details.
///

#include <iostream>
#include <json/json.hpp>
#include <random>
#include <string>

/// @brief Reads every value of the tree, to exercise the accessors.
static void visit(const json::jnode_t &node)
{
    switch (node.get_type()) {
    case json::JTYPE_ARRAY:
    case json::JTYPE_OBJECT:
        for (std::size_t i = 0; i < node.size(); ++i) {
            visit(node[i]);
        }
        break;
    case json::JTYPE_STRING:
        (void)node.as_string();
        break;
    case json::JTYPE_NUMBER:
        (void)node.as_number<int>();
        (void)node.as_number<double>();
        break;
    default:
        break;
    }
}

/// @brief Applies a few random edits to the string.
static void mutate(std::string &input, std::mt19937 &rng)
{
    static const char alphabet[] = "{}[],:\"'\\/*-+.eE0123456789tfnaulsrx \n#\xff\x80";
    unsigned edits               = 1U + static_cast<unsigned>(rng() % 6U);
    for (unsigned i = 0; i < edits && !input.empty(); ++i) {
        std::size_t position = rng() % input.size();
        char chr             = alphabet[rng() % (sizeof(alphabet) - 1)];
        switch (rng() % 4) {
        case 0:
            input.erase(position, 1 + rng() % 3);
            break;
        case 1:
            input.insert(input.begin() + static_cast<std::string::difference_type>(position), chr);
            break;
        case 2:
            input[position] = chr;
            break;
        default:
            input.resize(position);
            break;
        }
    }
}

int main(int, char *[])
{
    const char *seeds[] = {
        "{\"a\":1,\"b\":[1,2,{\"c\":null}],\"d\":\"x\\u00e9\\n\",e:true,'f':-1.5e3}",
        "[1,2,3,[4,[5,[6]]]] // comment\n",
        "{ /* c */ a: 'b', // x\n c: [ ], d: { } , }",
        "\"a\\\nb\"",
        "[true,false,null,\"\\ud83d\\ude00\"]",
    };
    const std::size_t seed_count = sizeof(seeds) / sizeof(seeds[0]);

    std::mt19937 rng(12345);
    unsigned accepted = 0;
    for (unsigned iteration = 0; iteration < 20000; ++iteration) {
        std::string input = seeds[rng() % seed_count];
        mutate(input, rng);
        try {
            json::jnode_t root = json::parser::parse(input);
            visit(root);
            (void)root.to_string(true);
            (void)root.to_string(false);
            ++accepted;
        } catch (const json::parser_error &) {
            // Rejecting the input is fine.
        }
    }
    // Make sure the mutations do not turn every document into garbage.
    if (accepted == 0) {
        std::cerr << "No mutated document was accepted, the test is not exercising the parser.\n";
        return 1;
    }
    return 0;
}
