/// @file test_string.cpp
/// @author Enrico Fraccaroli (enry.frak@gmail.com)
/// @brief Tests the decoding of escape sequences inside strings.
///
/// @copyright (c) 2024 This file is distributed under the MIT License.
/// See LICENSE.md for details.
///

#include <iostream>
#include <json/json.hpp>
#include <string>

/// @brief Parses `[ "<json_string>" ]` and checks the decoded value.
static bool expect_decoded(const std::string &json_string, const std::string &expected)
{
    try {
        json::jnode_t root = json::parser::parse("[\"" + json_string + "\"]");
        std::string value  = root[0].as_string();
        if (value != expected) {
            std::cerr << "Decoding `" << json_string << "` gave a different result.\n";
            return false;
        }
    } catch (const json::parser_error &e) {
        std::cerr << "Decoding `" << json_string << "` failed: " << e.what() << "\n";
        return false;
    }
    return true;
}

/// @brief Parses `[ "<json_string>" ]` and checks that it is rejected.
static bool expect_invalid(const std::string &json_string)
{
    try {
        json::parser::parse("[\"" + json_string + "\"]");
    } catch (const json::parser_error &) {
        return true;
    }
    std::cerr << "String `" << json_string << "` was accepted, but it should be rejected.\n";
    return false;
}

static bool test_simple_escapes()
{
    return expect_decoded("a\\nb", "a\nb") && expect_decoded("a\\tb", "a\tb") && expect_decoded("a\\\\b", "a\\b") &&
           expect_decoded("a\\/b", "a/b") && expect_decoded("\\b\\f\\r", "\b\f\r") && expect_decoded("plain", "plain");
}

static bool test_line_continuations()
{
    return expect_decoded("a\\\nb", "a\nb") && expect_decoded("a\\ \t\nb", "a\nb") &&
           expect_decoded("a\\\r\nb", "a\nb") &&
           // An escaped backslash followed by a line break is not a continuation.
           expect_decoded("a\\\\\nb", "a\\\nb");
}

static bool test_unicode_escapes()
{
    // One, two, three and four bytes in UTF-8.
    return expect_decoded("\\u0041", "A") && expect_decoded("\\u00e9", "\xC3\xA9") &&
           expect_decoded("\\u00E9", "\xC3\xA9") && expect_decoded("\\u20AC", "\xE2\x82\xAC") &&
           expect_decoded("\\uD83D\\uDE00", "\xF0\x9F\x98\x80") && expect_decoded("x\\u0041y", "xAy");
}

static bool test_invalid_unicode_escapes()
{
    const char *bad[] = { "\\u", "\\u1", "\\u12", "\\u123", "\\uZZZZ", "\\u12G4", "\\uD83D", "\\uD83Dx", "\\uDE00",
                          "\\uD83D\\u0041" };
    for (std::size_t i = 0; i < sizeof(bad) / sizeof(bad[0]); ++i) {
        if (!expect_invalid(bad[i])) {
            return false;
        }
    }
    return true;
}

int main(int, char *[])
{
    if (!test_simple_escapes()) {
        return 1;
    }
    if (!test_line_continuations()) {
        return 1;
    }
    if (!test_unicode_escapes()) {
        return 1;
    }
    if (!test_invalid_unicode_escapes()) {
        return 1;
    }
    return 0;
}
