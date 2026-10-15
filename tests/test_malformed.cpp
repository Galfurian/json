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
    const char *inputs[] = {
        "",
        "   ",
        "\n",
        "/*",
        "//",
        "/* unterminated",
        "[",
        "{",
        "[1,",
        "{\"a\"",
        "{\"a\":",
        "{\"a\":1,",
        "[[[[",
        "{\"a\":[",
        "// only a comment",
        "/* only a comment */"};
    for (std::size_t i = 0; i < sizeof(inputs) / sizeof(inputs[0]); ++i) {
        if (!expect_error(inputs[i])) {
            return false;
        }
    }
    return true;
}

static bool test_unterminated_strings()
{
    const char *inputs[] = {"\"abc", "'abc", "\"", "\"\\", "\"abc\\\"", "[\"abc]", "{\"a\": \"b}"};
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
    std::string ok          = std::string(limit, '[') + std::string(limit, ']');
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

static bool test_invalid_numbers()
{
    const char *bad[] = {"-",     "+",      ".",       "1e",    "1e+",  "1.2.3", "01",
                         "-01",   "--1",    "+-1",     "0x",    "0xG",  "0x1G",  "0x11111111111111111",
                         "[.e5]", "[1.5.]", "Infinit", "-NaNa", "[- 1]"};
    for (std::size_t i = 0; i < sizeof(bad) / sizeof(bad[0]); ++i) {
        if (!expect_error(bad[i])) {
            return false;
        }
    }
    const char *good[] = {
        "0", "-0", "10", "-1.5", "0.5", "1e5", "1E+5", "1e-5", "-1.5e10", "[1, -2, 3.5]",
        // The extra forms of JSON5.
        "+1", ".5", "-.5", "5.", "[1.e5]", "0x10", "-0xFF", "0Xab", "Infinity", "-Infinity", "+NaN", "NaN", "[1,2,3.]",
        "{Infinity: NaN, NaN: -Infinity}"};
    for (std::size_t i = 0; i < sizeof(good) / sizeof(good[0]); ++i) {
        if (!expect_ok(good[i])) {
            return false;
        }
    }
    return true;
}

static bool test_invalid_literals()
{
    // Bare words are only valid as object keys.
    const char *bad[] = {"nul", "tru", "fals", "truee", "nulll", "[nope]", "{a: hello}", "@", "[#]", "[1, undefined]"};
    for (std::size_t i = 0; i < sizeof(bad) / sizeof(bad[0]); ++i) {
        if (!expect_error(bad[i])) {
            return false;
        }
    }
    return expect_ok("[true,false,null]") && expect_ok("{true: 1, null: 2, a_b$1: 3}");
}

static bool test_non_ascii_bytes()
{
    // Bytes above 0x7F are negative when char is signed. They must not reach the
    // <cctype> functions unconverted, that is undefined behavior and it aborts
    // the debug runtime of MSVC.
    return expect_error("\xff") && expect_error("[\x80]") && expect_error("\xe9\xe8") && expect_ok("[\"\xc3\xa9\"]") &&
           expect_ok("{\xc3\xa9: 1}") && expect_error("[1\xff]");
}

static bool test_invalid_structure()
{
    const char *bad[] = {
        "[1 2]",    "[1,,2]",  "[,]",      "{,}", "{\"a\":1,,}", "{\"a\" 1}", "{\"a\":1 \"b\":2}", "{1:2}",
        "{[1]: 2}", "{\"a\"}", "{\"a\":}", "[}",  "{]",          "[1}",       "{\"a\":1]",         "{\"a\":1,\"b\""};
    for (std::size_t i = 0; i < sizeof(bad) / sizeof(bad[0]); ++i) {
        if (!expect_error(bad[i])) {
            return false;
        }
    }
    // JSON5 conveniences stay supported.
    return expect_ok("[1, 2, ]") && expect_ok("{a: 1, 'b': 2, \"c\": 3,}") && expect_ok("{} /* c */") &&
           expect_ok("[1 /* a */, /* b */ 2 // c\n]");
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
    if (!test_invalid_numbers()) {
        return 1;
    }
    if (!test_invalid_literals()) {
        return 1;
    }
    if (!test_non_ascii_bytes()) {
        return 1;
    }
    if (!test_invalid_structure()) {
        return 1;
    }
    return 0;
}
