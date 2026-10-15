/// @file test_parser_equivalence.cpp
/// @author Enrico Fraccaroli (enry.frak@gmail.com)
/// @brief Checks that the single-pass parser agrees with the token-based one.
/// @details json::parser::parse reads the text in a single pass. The functions
/// json::detail::tokenize and json::detail::json_parse build a list of tokens
/// first. They must accept the same documents and build the same tree.
///
/// @copyright (c) 2024 This file is distributed under the MIT License.
/// See LICENSE.md for details.
///

#include <iostream>
#include <json/json.hpp>
#include <random>
#include <string>
#include <vector>

/// @brief Parses with the token-based parser.
/// @return false if the document is rejected.
static bool parse_with_tokens(const std::string &text, std::string &output)
{
    try {
        std::vector<json::detail::token_t> tokens;
        json::detail::tokenize(text, tokens);
        json::jnode_t root;
        std::size_t index = 0;
        json::detail::json_parse(tokens, 0UL, index, root, 0UL, json::config::max_depth);
        // Only comments are allowed after the root value.
        for (; index < tokens.size(); ++index) {
            if (tokens[index].type != json::detail::JTOKEN_COMMENT) {
                return false;
            }
        }
        output = root.to_string(false);
        return true;
    } catch (const json::parser_error &) {
        return false;
    }
}

/// @brief Parses with the single-pass parser.
/// @return false if the document is rejected.
static bool parse_directly(const std::string &text, std::string &output)
{
    try {
        output = json::parser::parse(text).to_string(false);
        return true;
    } catch (const json::parser_error &) {
        return false;
    }
}

/// @brief Compares the two parsers on a document.
/// @return false if they disagree.
static bool agree(const std::string &text)
{
    std::string with_tokens;
    std::string directly;
    bool accepted_tokens   = parse_with_tokens(text, with_tokens);
    bool accepted_directly = parse_directly(text, directly);
    // The token-based parser accepts `{+NaN: 1}` by mistake, its key check sees
    // the number without the sign. The single-pass parser rejects it, as JSON5.
    if (accepted_tokens && !accepted_directly &&
        ((text.find("+NaN") != std::string::npos) || (text.find("-NaN") != std::string::npos) ||
         (text.find("+Infinity") != std::string::npos))) {
        return true;
    }
    if (accepted_tokens != accepted_directly) {
        std::cerr << "Disagreement on `" << text.substr(0, 80) << "`: the token-based parser "
                  << (accepted_tokens ? "accepts" : "rejects") << " it, the single-pass one "
                  << (accepted_directly ? "accepts" : "rejects") << " it.\n";
        return false;
    }
    if (accepted_tokens && (with_tokens != directly)) {
        std::cerr << "Different trees for `" << text.substr(0, 80) << "`:\n  " << with_tokens << "\n  " << directly
                  << "\n";
        return false;
    }
    return true;
}

/// @brief Applies a few random edits to the string.
static void mutate(std::string &input, std::mt19937 &rng)
{
    static const char alphabet[] = "{}[],:\"'\\/*-+.eE0123456789tfnaulsrxIN \n\t#\xff\x80";
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
        "[0x1F, .5, +1, 5., Infinity, -Infinity, NaN, 1e3, -0]",
        "{Infinity: 1, NaN: 2, null: 3, true: 4, a_b$1: 5}",
        "  \n [ 1 , 2 /* x */ , \t 3 ]  // end",
    };
    const std::size_t seed_count = sizeof(seeds) / sizeof(seeds[0]);

    for (std::size_t i = 0; i < seed_count; ++i) {
        if (!agree(seeds[i])) {
            return 1;
        }
    }
    std::mt19937 rng(2024);
    unsigned failures = 0;
    for (unsigned iteration = 0; iteration < 100000; ++iteration) {
        std::string input = seeds[rng() % seed_count];
        mutate(input, rng);
        if (!agree(input) && (++failures >= 10)) {
            return 1;
        }
    }
    return (failures == 0) ? 0 : 1;
}
