/// @file test_malformed.cpp
/// @author Enrico Fraccaroli (enry.frak@gmail.com)
/// @brief Tests that the parser rejects malformed or hostile input.
///
/// @copyright (c) 2024 This file is distributed under the MIT License.
/// See LICENSE.md for details.
///

#include <iostream>
#include <json/json.hpp>
#include <string>

/// @brief Checks that parsing the input throws a json::parser_error.
static bool expect_error(const std::string &input)
{
    try {
        json::parser::parse(input);
    } catch (const json::parser_error &) {
        return true;
    }
    std::cerr << "Input was accepted, but it should be rejected: `" << input.substr(0, 40) << "`\n";
    return false;
}

/// @brief Checks that parsing the input succeeds.
static bool expect_ok(const std::string &input)
{
    try {
        json::parser::parse(input);
    } catch (const json::parser_error &e) {
        std::cerr << "Input was rejected, but it should be accepted: `" << input.substr(0, 40) << "` : " << e.what()
                  << "\n";
        return false;
    }
    return true;
}

static bool test_truncated_input()
{
    const char *inputs[] = { "",       "   ",     "\n",    "/*",    "//",         "/* unterminated",
                             "[",      "{",       "[1,",   "{\"a\"", "{\"a\":",    "{\"a\":1,",
                             "[[[[",   "{\"a\":[", "// only a comment", "/* only a comment */" };
    for (std::size_t i = 0; i < sizeof(inputs) / sizeof(inputs[0]); ++i) {
        if (!expect_error(inputs[i])) {
            return false;
        }
    }
    return true;
}

static bool test_unterminated_strings()
{
    const char *inputs[] = { "\"abc", "'abc", "\"", "\"\\", "\"abc\\\"", "[\"abc]", "{\"a\": \"b}" };
    for (std::size_t i = 0; i < sizeof(inputs) / sizeof(inputs[0]); ++i) {
        if (!expect_error(inputs[i])) {
            return false;
        }
    }
    return true;
}

static bool test_trailing_data()
{
    return expect_error("{} {}") && expect_error("[1] 2") && expect_error("1 2") && expect_ok("{} // done") &&
           expect_ok("[1] /* done */");
}

static bool test_depth_limit()
{
    const std::size_t limit = json::config::max_depth;
    // Right at the limit.
    std::string ok = std::string(limit, '[') + std::string(limit, ']');
    if (!expect_ok(ok)) {
        return false;
    }
    // Way beyond the limit, this must not exhaust the stack.
    std::string deep = std::string(200000, '[') + std::string(200000, ']');
    if (!expect_error(deep)) {
        return false;
    }
    // Unbalanced and deep.
    return expect_error(std::string(200000, '['));
}

int main(int, char *[])
{
    if (!test_truncated_input()) {
        return 1;
    }
    if (!test_unterminated_strings()) {
        return 1;
    }
    if (!test_trailing_data()) {
        return 1;
    }
    if (!test_depth_limit()) {
        return 1;
    }
    return 0;
}
