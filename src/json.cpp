/// @file json.cpp
/// @author Enrico Fraccaroli (enry.frak@gmail.com)
/// @brief Implement the functionality of the jnode_t class.
///
/// @copyright (c) 2024 This file is distributed under the MIT License.
/// See LICENSE.md for details.
///

#include "json/json.hpp"

#include <cstdint>

/// @brief This namespace contains the main json_t class and stream functions.
namespace json
{

namespace config
{
bool strict_type_check          = false;
bool strict_existance_check     = false;
bool replace_escape_characters  = false;
char string_delimiter_character = '\'';
std::size_t max_depth           = 256;
std::size_t max_input_size      = 0;
} // namespace config

/// @brief Transforms the given JSON type to string.
/// @param type the JSON type to transform to string.
/// @return the string representing the JSON type.
auto jtype_to_string(jtype_t type) -> std::string
{
    if (type == JTYPE_STRING) {
        return "STRING";
    }
    if (type == JTYPE_OBJECT) {
        return "OBJECT";
    }
    if (type == JTYPE_ARRAY) {
        return "ARRAY";
    }
    if (type == JTYPE_BOOLEAN) {
        return "BOOLEAN";
    }
    if (type == JTYPE_NUMBER) {
        return "NUMBER";
    }
    if (type == JTYPE_NULL) {
        return "NULL";
    }
    if (type == JTYPE_ERROR) {
        return "ERROR";
    }
    return "UNKNOWN";
}

/// @brief Converts a token type to a string.
/// @param type The token type.
/// @return The string representation of the token type.
static auto token_type_to_string(detail::token_type_t type) -> std::string
{
    switch (type) {
    case detail::JTOKEN_UNKNOWN:
        return "UNKNOWN";
    case detail::JTOKEN_STRING:
        return "STRING";
    case detail::JTOKEN_NUMBER:
        return "NUMBER";
    case detail::JTOKEN_CURLY_OPEN:
        return "CURLY_OPEN";
    case detail::JTOKEN_CURLY_CLOSE:
        return "CURLY_CLOSE";
    case detail::JTOKEN_BRACKET_OPEN:
        return "BRACKET_OPEN";
    case detail::JTOKEN_BRACKET_CLOSE:
        return "BRACKET_CLOSE";
    case detail::JTOKEN_COMMA:
        return "COMMA";
    case detail::JTOKEN_COLON:
        return "COLON";
    case detail::JTOKEN_BOOLEAN:
        return "BOOLEAN";
    case detail::JTOKEN_COMMENT:
        return "COMMENT";
    case detail::JTOKEN_NULL:
        return "NULL";
    case detail::JTOKEN_IDENTIFIER:
        return "IDENTIFIER";
    default:
        return "INVALID";
    }
}

parser_error::parser_error(std::size_t _line, const std::string &_message)
    : std::runtime_error(_message)
    , line(_line)
{
    // Nothing to do.
}

type_error::type_error(std::size_t _line, json::jtype_t _expected, json::jtype_t _found)
    : json::parser_error(_line, build_message(_expected, _found))
    , expected(_expected)
    , found(_found)
{
    // Nothing to do.
}

auto type_error::build_message(json::jtype_t _expected, json::jtype_t _found) -> std::string
{
    std::stringstream stream;
    stream << "Trying to access item of type " << json::jtype_to_string(_expected) << ", but we found a "
           << json::jtype_to_string(_found) << ".";
    return stream.str();
}

range_error::range_error(std::size_t _line, std::size_t _index, std::size_t _size)
    : json::parser_error(_line, build_message(_index, _size))
    , index(_index)
    , size(_size)
{
    // Nothing to do.
}

auto range_error::build_message(std::size_t _index, std::size_t _size) -> std::string
{
    std::stringstream stream;
    stream << "Trying to access item at " << _index << " of " << _size << ".";
    return stream.str();
}

namespace detail
{

/// @brief Replaces all the occurences of WHAT with WITH, in INPUT.
/// @param input the string we want to manipulate.
/// @param what the string we want to replace.
/// @param with the string we use as replacement.
/// @return a reference to the input string.
static inline auto replace_all(std::string &input, const std::string &what, const std::string &with) -> std::string &
{
    std::size_t pos = 0;
    while ((pos = input.find(what, pos)) != std::string::npos) {
        input.replace(pos, what.size(), with);
        pos += with.size();
    }
    return input;
}

/// @brief Replaces all the occurences of WHAT with WITH, in INPUT.
/// @param input the string we want to manipulate.
/// @param what the char we want to replace.
/// @param with the string we use as replacement.
/// @return a reference to the input string.
static inline auto replace_all(std::string &input, char what, const std::string &with) -> std::string &
{
    std::size_t pos = 0;
    while ((pos = input.find(what, pos)) != std::string::npos) {
        input.replace(pos, 1U, with);
        pos += with.size();
    }
    return input;
}

/// @brief Transforms the boolean value to string.
/// @param value the boolean value.
/// @return the string representation of the boolean value.
static inline auto bool_to_string(bool value) -> std::string { return value ? "true" : "false"; }

/// @brief Transforms the ASCII integer representing the character into a string.
/// @param value the input character.
/// @return the output string.
template <typename T>
static inline auto char_to_string(T value) -> std::string
{
    std::stringstream stream;
    stream << static_cast<int>(value);
    return stream.str();
}

/// @brief Transforms the number to string.
/// @param value the input number.
/// @return the output string.
template <typename T>
static inline auto number_to_string(const T &value) -> std::string
{
    std::stringstream stream;
    stream << value;
    return stream.str();
}

/// @brief Generates the indenation.
/// @param depth depth of the indentation.
/// @param tabsize the number of character for each depth unit.
/// @return the output indentation as string.
static inline auto make_indentation(unsigned depth, unsigned tabsize = 4) -> std::string
{
    return std::string(static_cast<std::string::size_type>(depth * tabsize), ' ');
}

/// @brief Checks if the given character is a whitespace.
/// @details The cast avoids undefined behavior: the <cctype> functions only
/// accept values representable as unsigned char, while char may be negative.
/// @param chr the input character.
/// @return if the character is a whitespace.
static inline auto is_space(char chr) -> bool { return std::isspace(static_cast<unsigned char>(chr)) != 0; }

/// @brief Checks if the given character is a decimal digit.
/// @param chr the input character.
/// @return if the character is a decimal digit.
static inline auto is_digit(char chr) -> bool { return std::isdigit(static_cast<unsigned char>(chr)) != 0; }

/// @brief Checks if the given character is a newline.
/// @param c the input character.
/// @return if the character is a newline.
static inline auto isnewline(char chr) -> bool { return (chr == '\n') || (chr == '\r'); }

/// @brief Skips a quoted string in SOURCE starting from INDEX.
/// @param source The source string.
/// @param index The current index.
/// @return The index after the closing quote, or std::string::npos if the
/// string is not terminated.
static auto skip_quoted_string(const std::string &source, std::size_t index) -> std::size_t
{
    // Store the opening quote (' or ") and move to the next character
    char quote = source[index++];
    // Scan the string.
    for (; index < source.length(); ++index) {
        // An escape sequence consumes the next character, whatever it is.
        if (source[index] == '\\') {
            ++index;
        } else if (source[index] == quote) {
            return index + 1;
        }
    }
    // No closing quote was found.
    return std::string::npos;
}

/// @brief Skips a single-line comment in SOURCE starting from INDEX.
/// @param source The source string.
/// @param index The current index.
/// @return The index after the newline character.
static auto skip_single_line_comment(const std::string &source, std::size_t index) -> std::size_t
{
    for (; index < source.length(); ++index) {
        if (detail::isnewline(source[index])) {
            return index;
        }
    }
    // Return the current position if no newline is found.
    return index;
}

/// @brief Skips a multi-line comment in SOURCE starting from INDEX.
/// @param source The source string.
/// @param index The current index.
/// @return The index after the closing "*/", or std::string::npos if the
/// comment is not terminated.
static auto skip_multi_line_comment(const std::string &source, std::size_t index) -> std::size_t
{
    // Scan the string.
    for (std::size_t slength = source.length(); index < slength; ++index) {
        // Check if we've reached the closing comment.
        if ((source[index] == '*') && (index + 1 < slength) && (source[index + 1] == '/')) {
            return index + 2;
        }
    }
    // No closing comment was found.
    return std::string::npos;
}

/// @brief Skips the whitespaces starting from INDEX.
/// @param source the sources string.
/// @param index the index we start skipping from.
/// @param line_number the current line number.
/// @return the index of the next non-whitespace character.
static inline auto skip_whitespaces(const std::string &source, std::size_t index, std::size_t &line_number)
    -> std::size_t
{
    // Scan the string.
    for (std::size_t slength = source.length(); index < slength; ++index) {
        // Check if we've reached a newline.
        if (detail::isnewline(source[index])) {
            ++line_number;
        }
        // Check if we've reached a whitespace.
        if (!detail::is_space(source[index])) {
            return index;
        }
    }
    // Return the current position if no whitespace is found.
    return index;
}

/// @brief Searches for the next whitespace in SOURCE starting from INDEX.
/// @param source The source string.
/// @param index The starting index.
/// @return The index of the next whitespace.
static inline auto find_next_whitespace(const std::string &source, std::size_t index) -> std::size_t
{
    // Get the length of the source string.
    std::size_t slength = source.length();
    // Scan the string.
    while (index < slength) {
        if (source[index] == '"' || source[index] == '\'') {
            index = skip_quoted_string(source, index);
        } else if ((index + 1 < slength) && (source[index] == '/') && (source[index + 1] == '/')) {
            index = skip_single_line_comment(source, index + 2);
        } else if ((index + 1 < slength) && (source[index] == '/') && (source[index + 1] == '*')) {
            index = skip_multi_line_comment(source, index + 2);
        } else if (detail::is_space(source[index])) {
            return index;
        } else {
            ++index;
        }
        // An unterminated string or comment extends to the end of the source.
        // The error is reported when the token is processed.
        if (index == std::string::npos) {
            break;
        }
    }
    return slength;
}

/// @brief Parses four hexadecimal digits.
/// @param source The input string.
/// @param index The index of the first digit.
/// @param value The parsed value.
/// @return true if there are four valid hexadecimal digits at INDEX.
static auto parse_hex4(const std::string &source, std::size_t index, std::uint32_t &value) -> bool
{
    if (index + 4 > source.length()) {
        return false;
    }
    value = 0;
    for (std::size_t j = 0; j < 4; ++j) {
        char chr = source[index + j];
        value *= 16U;
        if (chr >= '0' && chr <= '9') {
            value += static_cast<std::uint32_t>(chr - '0');
        } else if (chr >= 'a' && chr <= 'f') {
            value += static_cast<std::uint32_t>(chr - 'a' + 10);
        } else if (chr >= 'A' && chr <= 'F') {
            value += static_cast<std::uint32_t>(chr - 'A' + 10);
        } else {
            return false;
        }
    }
    return true;
}

/// @brief Appends a Unicode code point to the string, encoded as UTF-8.
/// @param out The output string.
/// @param code_point The code point to append.
static void append_utf8(std::string &out, std::uint32_t code_point)
{
    if (code_point < 0x80U) {
        out.push_back(static_cast<char>(code_point));
    } else if (code_point < 0x800U) {
        out.push_back(static_cast<char>(0xC0U | (code_point >> 6U)));
        out.push_back(static_cast<char>(0x80U | (code_point & 0x3FU)));
    } else if (code_point < 0x10000U) {
        out.push_back(static_cast<char>(0xE0U | (code_point >> 12U)));
        out.push_back(static_cast<char>(0x80U | ((code_point >> 6U) & 0x3FU)));
        out.push_back(static_cast<char>(0x80U | (code_point & 0x3FU)));
    } else {
        out.push_back(static_cast<char>(0xF0U | (code_point >> 18U)));
        out.push_back(static_cast<char>(0x80U | ((code_point >> 12U) & 0x3FU)));
        out.push_back(static_cast<char>(0x80U | ((code_point >> 6U) & 0x3FU)));
        out.push_back(static_cast<char>(0x80U | (code_point & 0x3FU)));
    }
}

/// @brief Decodes a Unicode escape sequence, including surrogate pairs.
/// @param source The input string.
/// @param index The index of the backslash.
/// @param out The string the decoded code point is appended to.
/// @param offset The number of characters consumed from the source.
/// @return false if the sequence is not valid, in which case nothing is
/// appended.
static auto decode_unicode_escape(const std::string &source, std::size_t index, std::string &out, std::size_t &offset)
    -> bool
{
    std::uint32_t code_point = 0;
    if (!parse_hex4(source, index + 2, code_point)) {
        return false;
    }
    offset = 6;
    // A high surrogate must be followed by an escaped low surrogate.
    if (code_point >= 0xD800U && code_point <= 0xDBFFU) {
        std::uint32_t low = 0;
        if ((index + 11 < source.length()) && (source[index + 6] == '\\') && (source[index + 7] == 'u') &&
            parse_hex4(source, index + 8, low) && (low >= 0xDC00U && low <= 0xDFFFU)) {
            code_point = 0x10000U + ((code_point - 0xD800U) << 10U) + (low - 0xDC00U);
            offset     = 12;
        } else {
            return false;
        }
    } else if (code_point >= 0xDC00U && code_point <= 0xDFFFU) {
        // A lone low surrogate.
        return false;
    }
    append_utf8(out, code_point);
    return true;
}

/// @brief Decodes an escape sequence.
/// @param source The input string.
/// @param index The index of the backslash.
/// @param out The string the decoded character is appended to.
/// @param offset The number of characters consumed from the source.
/// @return false if the sequence is not valid. In that case the escaped
/// character is appended as it is.
static auto decode_escape_sequence(const std::string &source, std::size_t index, std::string &out, std::size_t &offset)
    -> bool
{
    offset = 2;
    switch (source[index + 1]) {
    case '\"':
    case '\'':
        out.push_back(config::string_delimiter_character);
        return true;
    case '\\':
        out.push_back('\\');
        return true;
    case '/':
        out.push_back('/');
        return true;
    case 'b':
        out.push_back('\b');
        return true;
    case 'f':
        out.push_back('\f');
        return true;
    case 'n':
        out.push_back('\n');
        return true;
    case 'r':
        out.push_back('\r');
        return true;
    case 't':
        out.push_back('\t');
        return true;
    case 'u':
        if (decode_unicode_escape(source, index, out, offset)) {
            return true;
        }
        offset = 2;
        out.push_back('u');
        return false;
    default:
        // Fallback for unexpected escape sequences.
        out.push_back(source[index + 1]);
        return true;
    }
}

/// @brief Extracts a number token.
/// @param source The input string.
/// @param index The starting index.
/// @return The index after the number.
static auto extract_number(const std::string &source, std::size_t index) -> std::size_t
{
    std::size_t slength = source.length();
    // Skip the sign.
    if ((source[index] == '-') || (source[index] == '+')) {
        ++index;
    }
    // Hexadecimal numbers.
    if ((index + 1 < slength) && (source[index] == '0') && ((source[index + 1] == 'x') || (source[index + 1] == 'X'))) {
        index += 2;
        while ((index < slength) && (std::isxdigit(static_cast<unsigned char>(source[index])) != 0)) {
            ++index;
        }
        return index;
    }
    // Skip the integer part.
    while (index < slength) {
        if (detail::is_digit(source[index]) || (source[index] == '.')) {
            ++index;
        } else if ((source[index] == 'e' || source[index] == 'E') && index + 1 < slength) {
            if ((source[index + 1] == '+') || (source[index + 1] == '-') || detail::is_digit(source[index + 1])) {
                index += 2;
            } else {
                break;
            }
        } else {
            break;
        }
    }
    return index;
}

/// @brief Deserializes the given string.
/// @param source The input string.
/// @param valid Set to false if the string contains an invalid escape sequence.
/// @return The deserialized input string.
static inline auto deserialize(const std::string &source, bool &valid) -> std::string
{
    std::size_t offset = 0;
    std::string out;
    valid = true;
    for (std::size_t index = 0; index < source.length(); ++index) {
        if (source[index] == '\\' && (index + 1) < source.length()) {
            if (!decode_escape_sequence(source, index, out, offset)) {
                valid = false;
            }
            index += offset - 1;
        } else {
            out.push_back(source[index]);
        }
    }
    return out;
}

/// @brief Deserializes the given string.
/// @param source The input string.
/// @return The deserialized input string.
static inline auto deserialize(const std::string &source) -> std::string
{
    bool valid = true;
    return deserialize(source, valid);
}

/// @brief Removes each escaped line break (a backslash, optional blanks and a
/// line break), as JSON5 requires.
/// @param input The raw string value.
/// @return The string with the line continuations replaced.
static auto replace_line_continuations(const std::string &input) -> std::string
{
    std::string output;
    output.reserve(input.size());
    for (std::size_t index = 0; index < input.size(); ++index) {
        if (input[index] != '\\') {
            output.push_back(input[index]);
            continue;
        }
        // Look for optional blanks followed by a line break.
        std::size_t next = index + 1;
        while ((next < input.size()) && ((input[next] == ' ') || (input[next] == '\t'))) {
            ++next;
        }
        if ((next < input.size()) && (input[next] == '\r') && (next + 1 < input.size()) && (input[next + 1] == '\n')) {
            ++next;
        }
        if ((next < input.size()) && (input[next] == '\n')) {
            index = next;
        } else {
            // Keep the escape sequence untouched, including the escaped character,
            // so that an escaped backslash is never mistaken for a continuation.
            output.push_back('\\');
            if (index + 1 < input.size()) {
                output.push_back(input[++index]);
            }
        }
    }
    return output;
}

/// @brief Checks if the given character can be part of an unquoted identifier.
/// @param chr the input character.
/// @return true if the character can be part of an identifier.
static inline auto is_identifier_char(char chr) -> bool
{
    return (std::isalnum(static_cast<unsigned char>(chr)) != 0) || (chr == '_') || (chr == '$') ||
           (static_cast<unsigned char>(chr) >= 0x80U);
}

/// @brief Checks if SOURCE contains the keyword LITERAL at INDEX, as a whole
/// word.
/// @param source The source string.
/// @param index The current index.
/// @param literal The keyword we are looking for.
/// @return true if the keyword is found and is not part of a longer identifier.
static inline auto is_literal_at(const std::string &source, std::size_t index, const char *literal) -> bool
{
    std::size_t length = std::char_traits<char>::length(literal);
    if (source.compare(index, length, literal) != 0) {
        return false;
    }
    return (index + length >= source.length()) || !is_identifier_char(source[index + length]);
}

/// @brief Checks if a number is a plain JSON number, already in canonical form.
/// @details This is the common case, and it does not allocate.
/// The grammar is -?(0|[1-9][0-9]*)(.[0-9]+)?(e[+-]?[0-9]+)?
/// @param number The string to check.
/// @return true if the string is a canonical number.
static auto is_canonical_number(const std::string &number) -> bool
{
    std::size_t index = 0;
    std::size_t size  = number.size();
    auto digits       = [&]() {
        std::size_t start = index;
        while ((index < size) && detail::is_digit(number[index])) {
            ++index;
        }
        return index - start;
    };
    if ((index < size) && (number[index] == '-')) {
        ++index;
    }
    std::size_t start = index;
    std::size_t count = digits();
    if ((count == 0) || ((count > 1) && (number[start] == '0'))) {
        return false;
    }
    if ((index < size) && (number[index] == '.')) {
        ++index;
        if (digits() == 0) {
            return false;
        }
    }
    if ((index < size) && (number[index] == 'e')) {
        ++index;
        if ((index < size) && ((number[index] == '+') || (number[index] == '-'))) {
            ++index;
        }
        if (digits() == 0) {
            return false;
        }
    }
    return index == size;
}

/// @brief Validates a number and rewrites it in a canonical form.
/// @details The accepted grammar is the one of JSON5, without the special
/// values: an optional sign, then either a hexadecimal number, or a decimal
/// number with an integer part, a fraction and an exponent, where the integer
/// part or the fraction may be missing (`.5`, `5.`) but not both. A leading zero
/// cannot be followed by other digits. The canonical form has no `+`, no
/// hexadecimal digits, a leading `0` before a bare fraction, and no dot without
/// digits after it.
/// @param number The string to check.
/// @param canonical The canonical form of the number.
/// @return true if the string is a valid number.
static auto normalize_number(const std::string &number, std::string &canonical) -> bool
{
    if (is_canonical_number(number)) {
        canonical = number;
        return true;
    }
    std::size_t index = 0;
    std::size_t size  = number.size();
    std::string sign;
    if ((index < size) && ((number[index] == '-') || (number[index] == '+'))) {
        if (number[index] == '-') {
            sign = "-";
        }
        ++index;
    }
    // Hexadecimal numbers are converted to decimal.
    if ((index + 1 < size) && (number[index] == '0') && ((number[index + 1] == 'x') || (number[index + 1] == 'X'))) {
        index += 2;
        // Sixteen digits always fit in 64 bits.
        if ((index >= size) || (size - index > 16)) {
            return false;
        }
        unsigned long long value = 0;
        for (; index < size; ++index) {
            char chr = number[index];
            value *= 16ULL;
            if (chr >= '0' && chr <= '9') {
                value += static_cast<unsigned long long>(chr - '0');
            } else if (chr >= 'a' && chr <= 'f') {
                value += static_cast<unsigned long long>(chr - 'a' + 10);
            } else if (chr >= 'A' && chr <= 'F') {
                value += static_cast<unsigned long long>(chr - 'A' + 10);
            } else {
                return false;
            }
        }
        canonical = sign + std::to_string(value);
        return true;
    }
    auto digits = [&]() {
        std::size_t start = index;
        while ((index < size) && detail::is_digit(number[index])) {
            ++index;
        }
        return number.substr(start, index - start);
    };
    // The integer part. A leading zero cannot be followed by other digits.
    std::string integer = digits();
    if ((integer.size() > 1) && (integer[0] == '0')) {
        return false;
    }
    // The fraction.
    std::string fraction;
    if ((index < size) && (number[index] == '.')) {
        ++index;
        fraction = digits();
    }
    if (integer.empty() && fraction.empty()) {
        return false;
    }
    // The exponent.
    std::string exponent;
    if ((index < size) && ((number[index] == 'e') || (number[index] == 'E'))) {
        ++index;
        exponent = "e";
        if ((index < size) && ((number[index] == '+') || (number[index] == '-'))) {
            exponent += number[index];
            ++index;
        }
        std::string exponent_digits = digits();
        if (exponent_digits.empty()) {
            return false;
        }
        exponent += exponent_digits;
    }
    if (index != size) {
        return false;
    }
    canonical = sign + (integer.empty() ? "0" : integer) + (fraction.empty() ? "" : "." + fraction) + exponent;
    return true;
}

/// @brief Processes a single substring.
/// @param source The substring.
/// @param tokens The token vector.
/// @param line_number The current line number.
static void process_token(const std::string &source, std::vector<token_t> &tokens, std::size_t line_number)
{
    // Get the length of the source string.
    std::size_t slength = source.length();
    // Initialize the index.
    std::size_t index   = 0;
    // Process the string.
    while (index < slength) {
        if ((index + 1 < slength) && (source[index] == '/') && (source[index + 1] == '/')) {
            // Find the end of the single line comment.
            std::size_t end = skip_single_line_comment(source, index + 2);
            // Add the comment to the tokens.
            tokens.emplace_back(source.substr(index, end - index), JTOKEN_COMMENT, line_number);
            // Skip the comment.
            index = end;
        } else if ((index + 1 < slength) && (source[index] == '/') && (source[index + 1] == '*')) {
            // Find the end of the multi line comment.
            std::size_t end = skip_multi_line_comment(source, index + 2);
            if (end == std::string::npos) {
                throw json::parser_error(
                    line_number + 1, "Error at line " + std::to_string(line_number + 1) + ": Unterminated comment.");
            }
            // Add the comment to the tokens.
            tokens.emplace_back(source.substr(index, end - index), JTOKEN_COMMENT, line_number);
            // Skip the comment.
            index = end;
        } else if ((source[index] == '"') || (source[index] == '\'')) {
            // Find the end of the quoted string.
            std::size_t end = skip_quoted_string(source, index);
            if (end == std::string::npos) {
                throw json::parser_error(
                    line_number + 1, "Error at line " + std::to_string(line_number + 1) + ": Unterminated string.");
            }
            // Add the string to the tokens.
            tokens.emplace_back(source.substr(index + 1, end - index - 2), JTOKEN_STRING, line_number);
            // Skip the string.
            index = end;
        } else if (source[index] == ',') {
            // Add the comma to the tokens.
            tokens.emplace_back(",", JTOKEN_COMMA, line_number);
            // Skip the comma.
            ++index;
        } else if ((source[index] == 't') && is_literal_at(source, index, "true")) {
            // Add the boolean to the tokens.
            tokens.emplace_back("true", JTOKEN_BOOLEAN, line_number);
            // Skip the boolean.
            index += 4;
        } else if ((source[index] == 'f') && is_literal_at(source, index, "false")) {
            // Add the boolean to the tokens.
            tokens.emplace_back("false", JTOKEN_BOOLEAN, line_number);
            // Skip the boolean.
            index += 5;
        } else if ((source[index] == 'n') && is_literal_at(source, index, "null")) {
            // Add the null to the tokens.
            tokens.emplace_back("null", JTOKEN_NULL, line_number);
            // Skip the null.
            index += 4;
        } else if (source[index] == '}') {
            // Add the closing curly brace to the tokens.
            tokens.emplace_back("}", JTOKEN_CURLY_CLOSE, line_number);
            // Skip the closing curly brace.
            ++index;
        } else if (source[index] == '{') {
            // Add the opening curly brace to the tokens.
            tokens.emplace_back("{", JTOKEN_CURLY_OPEN, line_number);
            // Skip the opening curly brace.
            ++index;
        } else if (source[index] == ']') {
            // Add the closing bracket to the tokens.
            tokens.emplace_back("]", JTOKEN_BRACKET_CLOSE, line_number);
            // Skip the closing bracket.
            ++index;
        } else if (source[index] == '[') {
            // Add the opening bracket to the tokens.
            tokens.emplace_back("[", JTOKEN_BRACKET_OPEN, line_number);
            // Skip the opening bracket.
            ++index;
        } else if (source[index] == ':') {
            // Add the colon to the tokens.
            tokens.emplace_back(":", JTOKEN_COLON, line_number);
            // Skip the colon.
            ++index;
        } else if (source[index] == ' ') {
            // Skip the whitespace.
            ++index;
        } else if (
            ((source[index] == 'I') && is_literal_at(source, index, "Infinity")) ||
            ((source[index] == 'N') && is_literal_at(source, index, "NaN"))) {
            // An unsigned special value.
            bool infinity = is_literal_at(source, index, "Infinity");
            tokens.emplace_back(infinity ? "Infinity" : "NaN", JTOKEN_NUMBER, line_number);
            index += infinity ? 8 : 3;
        } else if (
            (source[index] == '+' || source[index] == '-') &&
            (is_literal_at(source, index + 1, "Infinity") || is_literal_at(source, index + 1, "NaN"))) {
            // A signed special value.
            bool infinity   = is_literal_at(source, index + 1, "Infinity");
            std::string sgn = (source[index] == '-' && infinity) ? "-" : "";
            tokens.emplace_back(sgn + (infinity ? "Infinity" : "NaN"), JTOKEN_NUMBER, line_number);
            index += 1 + (infinity ? 8 : 3);
        } else if (
            source[index] == '-' || source[index] == '+' || source[index] == '.' || detail::is_digit(source[index])) {
            // Extract the number.
            std::size_t end    = extract_number(source, index);
            std::string number = source.substr(index, end - index);
            std::string canonical;
            if (!normalize_number(number, canonical)) {
                throw json::parser_error(
                    line_number + 1,
                    "Error at line " + std::to_string(line_number + 1) + ": Invalid number `" + number + "`.");
            }
            // Add the number to the tokens.
            tokens.emplace_back(canonical, JTOKEN_NUMBER, line_number);
            // Update the index.
            index = end;
        } else {
            // Extract the identifier.
            std::size_t end = index;
            while ((end < slength) && is_identifier_char(source[end])) {
                ++end;
            }
            // Anything else is not valid in a JSON document.
            if (end == index) {
                throw json::parser_error(
                    line_number + 1, "Error at line " + std::to_string(line_number + 1) + ": Unexpected character `" +
                                         std::string(1, source[index]) + "`.");
            }
            // Add the identifier to the tokens.
            tokens.emplace_back(source.substr(index, end - index), JTOKEN_IDENTIFIER, line_number);
            // Update the index.
            index = end;
        }
    }
}

/// @brief Tokenizes the given source string.
/// @param source The input source string.
/// @param tokens The output token vector.
/// @return The reference to the modified token vector.
auto tokenize(const std::string &source, std::vector<token_t> &tokens) -> std::vector<token_t> &
{
    // Most documents have at least one token every few bytes, avoid reallocating.
    tokens.reserve(tokens.size() + (source.size() / 8U) + 1U);
    // Keep track of the current line number.
    std::size_t line_number = 0;
    // Skip the whitespaces.
    std::size_t index       = detail::skip_whitespaces(source, 0, line_number);
    // Keep track of the next index.
    std::size_t next        = 0;
    // Process the tokens.
    while (index <= source.size()) {
        // Find the next whitespace.
        next = detail::find_next_whitespace(source, index);
        // If we reached the end of the string, we break.
        if (next == index) {
            break;
        }
        // Process the token.
        process_token(source.substr(index, next - index), tokens, line_number);
        // Skip the whitespaces.
        index = detail::skip_whitespaces(source, next, line_number);
    }
    return tokens;
}

/// @brief Skips comment tokens in the token list.
///
/// @details Advances the index past any token of type `JTOKEN_COMMENT`. The
/// index may end up equal to the number of tokens; use `peek` to access it.
///
/// @param tokens The list of tokens to parse.
/// @param index The current index in the token list, which will be updated.
auto skip_comments(const std::vector<token_t> &tokens, std::size_t &index) -> void
{
    while ((index < tokens.size()) && (tokens[index].type == JTOKEN_COMMENT)) {
        ++index;
    }
}

/// @brief Accesses a token, ensuring bounds are not exceeded.
///
/// @param tokens The list of tokens to parse.
/// @param index The index of the token to access.
/// @param line The line used for error reporting if the tokens are exhausted.
/// @return The token at the given index.
static auto peek(const std::vector<token_t> &tokens, std::size_t index, std::size_t line) -> const token_t &
{
    if (index >= tokens.size()) {
        throw json::parser_error(line, "Error at line " + std::to_string(line) + ": Unexpected end of input.");
    }
    return tokens[index];
}

/// @brief Skips a specified number of tokens, ensuring bounds are not exceeded.
///
/// @details Advances the index by the specified count. If this would exceed
/// the number of tokens, throws an error.
///
/// @param tokens The list of tokens to parse.
/// @param index The current index in the token list, which will be updated.
/// @param count The number of tokens to skip.
/// @param current The current JSON node being parsed, used for error reporting.
auto skip_tokens(const std::vector<token_t> &tokens, std::size_t &index, std::size_t count, jnode_t &current) -> void
{
    // Check if skipping the tokens would exceed the token list size.
    if ((index + count) >= tokens.size()) {
        throw json::parser_error(
            current.get_line_number(),
            "Error at line " + std::to_string(current.get_line_number()) + ": We ran out of tokens.");
    }
    // Advance the index by the specified count.
    index += count;
}

/// @brief Parses JSON tokens into a JSON node structure.
///
/// @details This function recursively parses a list of JSON tokens to build a
/// hierarchical JSON node structure representing objects, arrays, and values.
///
/// @param tokens The list of tokens to parse.
/// @param index The current index in the token list.
/// @param output_index The index to continue parsing from after this function returns.
/// @param current The current JSON node being built.
/// @return jnode_t& The constructed JSON node.
auto json_parse(
    std::vector<token_t> &tokens,
    std::size_t index,
    std::size_t &output_index,
    jnode_t &current,
    std::size_t depth,
    std::size_t max_depth) -> jnode_t &
{
    // Track the next index for recursive parsing.
    std::size_t next_index = 0;
    // Pointer to the key in key-value pairs.
    const char *key        = nullptr;
    // Skip any comment tokens before parsing.
    skip_comments(tokens, index);
    // Set the line number for error reporting.
    current.set_line_number(peek(tokens, index, 0).line_number + 1);
    // Refuse to recurse without bounds, we would exhaust the stack.
    if (depth > max_depth) {
        throw json::parser_error(
            current.get_line_number(),
            "Error at line " + std::to_string(current.get_line_number()) + ": Maximum nesting depth exceeded.");
    }
    // Parse JSON object if the token is an opening curly brace.
    if (peek(tokens, index, current.get_line_number()).type == JTOKEN_CURLY_OPEN) {
        // Skip the opening curly brace.
        skip_tokens(tokens, index, 1, current);
        // Set the current node type to object.
        current.set_type(JTYPE_OBJECT);
        // Parse key-value pairs until the closing curly brace is found.
        while (true) {
            // Skip comments before the key.
            skip_comments(tokens, index);
            const token_t &key_token = peek(tokens, index, current.get_line_number());
            // A closing curly brace ends the object, also after a trailing comma.
            if (key_token.type == JTOKEN_CURLY_CLOSE) {
                break;
            }
            // Only strings, identifiers and keywords are valid keys.
            const bool special_number =
                (key_token.type == JTOKEN_NUMBER) && ((key_token.value == "Infinity") || (key_token.value == "NaN"));
            if (!special_number && (key_token.type != JTOKEN_STRING) && (key_token.type != JTOKEN_IDENTIFIER) &&
                (key_token.type != JTOKEN_BOOLEAN) && (key_token.type != JTOKEN_NULL)) {
                throw json::parser_error(
                    key_token.line_number + 1,
                    "Error at line " + std::to_string(key_token.line_number + 1) + ": Expected an object key.");
            }
            // Set the key from the current token value.
            key = key_token.value.c_str();
            // Skip the key token.
            skip_tokens(tokens, index, 1, current);
            // Skip any comments before the colon.
            skip_comments(tokens, index);
            // Ensure the next token is a colon separating the key and value.
            if (peek(tokens, index, current.get_line_number()).type != JTOKEN_COLON) {
                throw json::parser_error(
                    current.get_line_number(),
                    "Error at line " + std::to_string(current.get_line_number()) + ": We did not find a COLON.");
            }
            // Skip the colon token.
            skip_tokens(tokens, index, 1, current);
            // Skip any comments before the value.
            skip_comments(tokens, index);
            // Track the index before parsing the value.
            next_index        = index;
            // Add a new property to the current object using the key.
            jnode_t &property = current.add_property(key);
            // Recursively parse the value.
            json_parse(tokens, index, next_index, property, depth + 1, max_depth);
            // Update the index after parsing the value.
            index = next_index;
            // Skip any comments after the value.
            skip_comments(tokens, index);
            // After a value we expect a comma or the end of the object.
            const token_t &separator = peek(tokens, index, current.get_line_number());
            if (separator.type == JTOKEN_COMMA) {
                skip_tokens(tokens, index, 1, current);
            } else if (separator.type != JTOKEN_CURLY_CLOSE) {
                throw json::parser_error(
                    separator.line_number + 1,
                    "Error at line " + std::to_string(separator.line_number + 1) + ": Expected `,` or `}`.");
            }
        }
    }
    // Parse JSON array if the token is an opening bracket.
    else if (peek(tokens, index, current.get_line_number()).type == JTOKEN_BRACKET_OPEN) {
        // Skip the opening bracket.
        skip_tokens(tokens, index, 1, current);
        // Set the current node type to array.
        current.set_type(JTYPE_ARRAY);
        // Parse array elements until the closing bracket is found.
        while (true) {
            // Skip comments before the element.
            skip_comments(tokens, index);
            // A closing bracket ends the array, also after a trailing comma.
            if (peek(tokens, index, current.get_line_number()).type == JTOKEN_BRACKET_CLOSE) {
                break;
            }
            // Track the index before parsing the element.
            next_index       = index;
            // Add a new element to the current array.
            jnode_t &element = current.add_element();
            // Recursively parse the array element.
            json_parse(tokens, index, next_index, element, depth + 1, max_depth);
            // Update the index after parsing the element.
            index = next_index;
            // Skip any comments after the element.
            skip_comments(tokens, index);
            // After an element we expect a comma or the end of the array.
            const token_t &separator = peek(tokens, index, current.get_line_number());
            if (separator.type == JTOKEN_COMMA) {
                skip_tokens(tokens, index, 1, current);
            } else if (separator.type != JTOKEN_BRACKET_CLOSE) {
                throw json::parser_error(
                    separator.line_number + 1,
                    "Error at line " + std::to_string(separator.line_number + 1) + ": Expected `,` or `]`.");
            }
        }
    }
    // Parse JSON primitive values (number, string, boolean, null).
    else if (peek(tokens, index, current.get_line_number()).type == JTOKEN_NUMBER) {
        current.set_type(JTYPE_NUMBER);
        current.set_value(peek(tokens, index, current.get_line_number()).value);
    } else if (peek(tokens, index, current.get_line_number()).type == JTOKEN_STRING) {
        current.set_type(JTYPE_STRING);
        const std::string &raw = peek(tokens, index, current.get_line_number()).value;
        // Most strings have no escape sequence, there is nothing to check.
        if (raw.find('\\') == std::string::npos) {
            current.set_value(raw);
        } else {
            // Make sure the escape sequences are valid.
            bool valid_escapes = true;
            deserialize(raw, valid_escapes);
            if (!valid_escapes) {
                throw json::parser_error(
                    current.get_line_number(),
                    "Error at line " + std::to_string(current.get_line_number()) + ": Invalid escape sequence.");
            }
            // Remove the escaped line breaks.
            current.set_value(replace_line_continuations(raw));
        }
    } else if (peek(tokens, index, current.get_line_number()).type == JTOKEN_BOOLEAN) {
        current.set_type(JTYPE_BOOLEAN);
        current.set_value(peek(tokens, index, current.get_line_number()).value);
    } else if (peek(tokens, index, current.get_line_number()).type == JTOKEN_NULL) {
        current.set_type(JTYPE_NULL);
        current.set_value("null");
    } else {
        // Throw an error if the token type is unrecognized.
        throw json::parser_error(
            current.get_line_number(), "Error at line " + std::to_string(current.get_line_number()) +
                                           ": Cannot type the entry `" + tokens[index].value + "`.");
    }
    // Move to the next token and update the output index.
    output_index = index + 1;
    return current;
}

/// @brief Parses a document in a single pass, without building a list of tokens.
/// @details It accepts the same language as tokenize followed by json_parse, and
/// builds the same tree, but it needs memory only for the tree.
class direct_parser
{
public:
    /// @brief Builds a parser for the given text.
    /// @param _source The text to parse. It must outlive the parser.
    /// @param _max_depth The maximum nesting depth of objects and arrays.
    direct_parser(const std::string &_source, std::size_t _max_depth)
        : source(_source)
        , max_depth(_max_depth)
    {
        // Nothing to do.
    }

    /// @brief Parses the whole text.
    /// @return The root of the tree.
    auto parse() -> jnode_t
    {
        jnode_t root;
        parse_value(root, 0);
        skip_blanks();
        if (position < source.size()) {
            fail("Unexpected data after the root value.");
        }
        return root;
    }

private:
    /// The text we are parsing.
    const std::string &source;
    /// The maximum nesting depth.
    std::size_t max_depth;
    /// The index of the next character.
    std::size_t position = 0;
    /// The line of the next character, starting from 1.
    std::size_t line     = 1;

    /// @brief Throws a parser_error for the current line.
    /// @param message What went wrong.
    [[noreturn]]
    void fail(const std::string &message) const
    {
        throw json::parser_error(line, "Error at line " + std::to_string(line) + ": " + message);
    }

    /// @brief Skips whitespaces and comments, and counts the lines.
    void skip_blanks()
    {
        const std::size_t size = source.size();
        while (position < size) {
            const char chr = source[position];
            if (chr == '\n') {
                ++line;
                ++position;
            } else if (is_space(chr)) {
                ++position;
            } else if ((chr == '/') && (position + 1 < size) && (source[position + 1] == '/')) {
                position += 2;
                while ((position < size) && !isnewline(source[position])) {
                    ++position;
                }
            } else if ((chr == '/') && (position + 1 < size) && (source[position + 1] == '*')) {
                position += 2;
                bool closed = false;
                while (position < size) {
                    if ((source[position] == '*') && (position + 1 < size) && (source[position + 1] == '/')) {
                        position += 2;
                        closed = true;
                        break;
                    }
                    if (source[position] == '\n') {
                        ++line;
                    }
                    ++position;
                }
                if (!closed) {
                    fail("Unterminated comment.");
                }
            } else {
                break;
            }
        }
    }

    /// @brief Skips the blanks and returns the next character.
    /// @return The next character, it fails if the text is over.
    auto next_character() -> char
    {
        skip_blanks();
        if (position >= source.size()) {
            fail("Unexpected end of input.");
        }
        return source[position];
    }

    /// @brief Reads a quoted string, and moves after the closing quote.
    /// @return The content of the string, as it is written in the text.
    auto read_quoted() -> std::string
    {
        const char quote        = source[position];
        const std::size_t start = position + 1;
        for (std::size_t index = start; index < source.size(); ++index) {
            if (source[index] == '\\') {
                // The escaped character cannot close the string.
                ++index;
            } else if (source[index] == quote) {
                std::string raw = source.substr(start, index - start);
                for (std::size_t i = 0; i < raw.size(); ++i) {
                    if (raw[i] == '\n') {
                        ++line;
                    }
                }
                position = index + 1;
                return raw;
            }
        }
        fail("Unterminated string.");
    }

    /// @brief Reads a word made of identifier characters.
    /// @return The word, it is empty if there is none.
    auto read_word() -> std::string
    {
        const std::size_t start = position;
        while ((position < source.size()) && is_identifier_char(source[position])) {
            ++position;
        }
        return source.substr(start, position - start);
    }

    /// @brief Reads the key of a property.
    /// @return The key.
    auto read_key() -> std::string
    {
        const char chr = next_character();
        if ((chr == '"') || (chr == '\'')) {
            return read_quoted();
        }
        // An identifier, but also a keyword: the keys do not start with a digit.
        if (!detail::is_digit(chr)) {
            std::string word = read_word();
            if (!word.empty()) {
                return word;
            }
        }
        fail("Expected an object key.");
    }

    /// @brief Parses an object, the current character is the opening brace.
    /// @param node The node that receives the object.
    /// @param depth The nesting depth of the node.
    void parse_object(jnode_t &node, std::size_t depth)
    {
        ++position;
        node.set_type(JTYPE_OBJECT);
        while (true) {
            // A closing brace ends the object, also after a trailing comma.
            if (next_character() == '}') {
                ++position;
                return;
            }
            std::string key = read_key();
            if (next_character() != ':') {
                fail("We did not find a COLON.");
            }
            ++position;
            parse_value(node.add_property(key), depth + 1);
            // After a value we expect a comma or the end of the object.
            const char separator = next_character();
            if (separator == ',') {
                ++position;
            } else if (separator != '}') {
                fail("Expected `,` or `}`.");
            }
        }
    }

    /// @brief Parses an array, the current character is the opening bracket.
    /// @param node The node that receives the array.
    /// @param depth The nesting depth of the node.
    void parse_array(jnode_t &node, std::size_t depth)
    {
        ++position;
        node.set_type(JTYPE_ARRAY);
        while (true) {
            // A closing bracket ends the array, also after a trailing comma.
            if (next_character() == ']') {
                ++position;
                return;
            }
            parse_value(node.add_element(), depth + 1);
            // After an element we expect a comma or the end of the array.
            const char separator = next_character();
            if (separator == ',') {
                ++position;
            } else if (separator != ']') {
                fail("Expected `,` or `]`.");
            }
        }
    }

    /// @brief Parses a string value, the current character is the quote.
    /// @param node The node that receives the string.
    void parse_string(jnode_t &node)
    {
        std::string raw = read_quoted();
        node.set_type(JTYPE_STRING);
        // Most strings have no escape sequence, there is nothing to check.
        if (raw.find('\\') == std::string::npos) {
            node.set_value(raw);
            return;
        }
        // Make sure the escape sequences are valid.
        bool valid_escapes = true;
        deserialize(raw, valid_escapes);
        if (!valid_escapes) {
            fail("Invalid escape sequence.");
        }
        // Remove the escaped line breaks.
        node.set_value(replace_line_continuations(raw));
    }

    /// @brief Parses a number, including the special values of JSON5.
    /// @param node The node that receives the number.
    void parse_number(jnode_t &node)
    {
        node.set_type(JTYPE_NUMBER);
        // A signed special value.
        if ((source[position] == '+') || (source[position] == '-')) {
            const bool negative = source[position] == '-';
            if (is_literal_at(source, position + 1, "Infinity")) {
                node.set_value(negative ? "-Infinity" : "Infinity");
                position += 1 + 8;
                return;
            }
            if (is_literal_at(source, position + 1, "NaN")) {
                node.set_value("NaN");
                position += 1 + 3;
                return;
            }
        }
        const std::size_t end = extract_number(source, position);
        const std::string number(source, position, end - position);
        std::string canonical;
        if (!normalize_number(number, canonical)) {
            fail("Invalid number `" + number + "`.");
        }
        node.set_value(canonical);
        position = end;
    }

    /// @brief Parses a value.
    /// @param node The node that receives the value.
    /// @param depth The nesting depth of the node.
    void parse_value(jnode_t &node, std::size_t depth)
    {
        const char chr = next_character();
        node.set_line_number(line);
        // Refuse to recurse without bounds, we would exhaust the stack.
        if (depth > max_depth) {
            fail("Maximum nesting depth exceeded.");
        }
        if (chr == '{') {
            parse_object(node, depth);
        } else if (chr == '[') {
            parse_array(node, depth);
        } else if ((chr == '"') || (chr == '\'')) {
            parse_string(node);
        } else if ((chr == '-') || (chr == '+') || (chr == '.') || detail::is_digit(chr)) {
            parse_number(node);
        } else if (is_identifier_char(chr)) {
            const std::string word = read_word();
            if (word == "true" || word == "false") {
                node.set_type(JTYPE_BOOLEAN);
                node.set_value(word);
            } else if (word == "null") {
                node.set_type(JTYPE_NULL);
                node.set_value("null");
            } else if (word == "Infinity" || word == "NaN") {
                node.set_type(JTYPE_NUMBER);
                node.set_value(word);
            } else {
                fail("Cannot type the entry `" + word + "`.");
            }
        } else {
            fail("Unexpected character `" + std::string(1, chr) + "`.");
        }
    }
};

} // namespace detail

/// @brief Contains parsing functions.
namespace parser
{

auto parse(const std::string &json_string) -> jnode_t { return parse(json_string, parse_options()); }

auto parse(const std::string &json_string, const parse_options &options) -> jnode_t
{
    if ((options.max_input_size != 0) && (json_string.size() > options.max_input_size)) {
        throw json::parser_error(
            0, "Error: The input is " + std::to_string(json_string.size()) + " bytes long, the limit is " +
                   std::to_string(options.max_input_size) + ".");
    }
    return detail::direct_parser(json_string, options.max_depth).parse();
}

auto read_file(const std::string &filename, std::string &content) -> bool
{
    std::ifstream file(filename.c_str());
    if (!file.is_open()) {
        return false;
    }
    std::stringstream stream;
    stream << file.rdbuf() << " ";
    content.append(stream.str());
    file.close();
    return true;
}

auto parse_file(const std::string &filename) -> jnode_t { return parse_file(filename, parse_options()); }

auto parse_file(const std::string &filename, const parse_options &options) -> jnode_t
{
    std::ifstream file(filename.c_str(), std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        throw json::parser_error(0, "Error: Cannot open the file `" + filename + "`.");
    }
    // Check the size before loading the file in memory.
    if (options.max_input_size != 0) {
        file.seekg(0, std::ios::end);
        std::streamoff size = file.tellg();
        file.seekg(0, std::ios::beg);
        if ((size >= 0) && (static_cast<std::size_t>(size) > options.max_input_size)) {
            throw json::parser_error(
                0, "Error: The file `" + filename + "` is " + std::to_string(size) + " bytes long, the limit is " +
                       std::to_string(options.max_input_size) + ".");
        }
    }
    std::stringstream stream;
    stream << file.rdbuf();
    return parse(stream.str(), options);
}

auto write_file(const std::string &filename, const jnode_t &node, bool pretty, unsigned tabsize) -> bool
{
    std::ofstream out(filename.c_str());
    if (out.is_open()) {
        out << node.to_string(pretty, tabsize);
        out.close();
        return true;
    }
    return false;
}

} // namespace parser

jnode_t::jnode_t()
    : type(JTYPE_NULL)
    , line_number()

{
    // Nothing to do.
}

jnode_t::jnode_t(jtype_t _type)
    : type(_type)
    , line_number()

{
    // Nothing to do.
}

auto jnode_t::get_value() const -> std::string { return value; }

auto jnode_t::get_type() const -> jtype_t { return type; }

auto jnode_t::is_string() const -> bool { return type == JTYPE_STRING; }

auto jnode_t::is_bool() const -> bool { return type == JTYPE_BOOLEAN; }

auto jnode_t::is_array() const -> bool { return type == JTYPE_ARRAY; }

auto jnode_t::is_object() const -> bool { return type == JTYPE_OBJECT; }

auto jnode_t::is_number() const -> bool { return type == JTYPE_NUMBER; }

auto jnode_t::is_null() const -> bool { return type == JTYPE_NULL; }

auto jnode_t::get_line_number() const -> std::size_t { return line_number; }

auto jnode_t::size() const -> std::size_t
{
    if (type == JTYPE_ARRAY) {
        return arr.size();
    }
    if (type == JTYPE_OBJECT) {
        return properties.size();
    }
    return 0;
}

auto jnode_t::has_property(const std::string &key) const -> bool
{
    if (type == JTYPE_OBJECT) {
        return properties.find(key) != properties.end();
    }
    return false;
}

auto jnode_t::as_bool() const -> bool
{
    if (type == JTYPE_BOOLEAN) {
        return value == "true";
    }
    if (json::config::strict_type_check) {
        throw json::type_error(line_number, JTYPE_BOOLEAN, type);
    }
    return false;
}

auto jnode_t::as_string() const -> std::string
{
    if (type == JTYPE_STRING) {
        return detail::deserialize(value);
    }
    if (json::config::strict_type_check) {
        throw json::type_error(line_number, JTYPE_STRING, type);
    }
    return {};
}

auto jnode_t::set_type(jtype_t _type) -> jnode_t &
{
    type = _type;
    return *this;
}

auto jnode_t::set_value(const std::string &_value) -> jnode_t &
{
    if ((type != JTYPE_OBJECT) && (type != JTYPE_ARRAY)) {
        value = _value;
    } else {
        throw json::parser_error(line_number, "Trying to set the value of a " + json::jtype_to_string(type) + " node.");
    }
    return *this;
}

auto jnode_t::set_line_number(std::size_t _line_number) -> jnode_t &
{
    line_number = _line_number;
    return *this;
}

auto jnode_t::add_property(const std::string &key) -> jnode_t &
{
    if (type != JTYPE_OBJECT) {
        throw json::parser_error(
            line_number, "Trying to add a property to a " + json::jtype_to_string(type) + " node.");
    }
    return properties.set(key, jnode_t())->second;
}

auto jnode_t::add_property(const std::string &key, const jnode_t &node) -> jnode_t &
{
    if (type != JTYPE_OBJECT) {
        throw json::parser_error(
            line_number, "Trying to add a property to a " + json::jtype_to_string(type) + " node.");
    }
    return properties.set(key, node)->second;
}

void jnode_t::remove_property(const std::string &key)
{
    if (type != JTYPE_OBJECT) {
        throw json::parser_error(
            line_number, "Trying to remove a property from a " + json::jtype_to_string(type) + " node.");
    }
    properties.erase(key);
}

auto jnode_t::add_element(const jnode_t &node) -> jnode_t &
{
    if (type != JTYPE_ARRAY) {
        throw json::parser_error(
            line_number, "Trying to add an element to a " + json::jtype_to_string(type) + " node.");
    }
    arr.push_back(node);
    return arr.back();
}

void jnode_t::remove_element(std::size_t index)
{
    if (type != JTYPE_ARRAY) {
        throw json::parser_error(
            line_number, "Trying to add an element to a " + json::jtype_to_string(type) + " node.");
    }
    if (index >= arr.size()) {
        throw json::range_error(line_number, index, arr.size());
    }
    arr.erase(arr.begin() + static_cast<std::ptrdiff_t>(index));
}

void jnode_t::reserve(std::size_t size)
{
    if (type != JTYPE_ARRAY) {
        throw json::parser_error(line_number, "Trying to reserve space in a " + json::jtype_to_string(type) + " node.");
    }
    arr.reserve(size);
}

void jnode_t::resize(std::size_t size)
{
    if (type != JTYPE_ARRAY) {
        throw json::parser_error(line_number, "Trying to reserve space in a " + json::jtype_to_string(type) + " node.");
    }
    arr.resize(size);
}

void jnode_t::clear()
{
    // We set the type to error, so that the node must be properly set again to
    // make the tree coherent again.
    type = JTYPE_ERROR;
    value.clear();
    properties.clear();
    arr.clear();
}

auto jnode_t::operator[](std::size_t index) const -> const jnode_t &
{
    if (type == JTYPE_ARRAY) {
        if (index >= arr.size()) {
            throw json::range_error(line_number, index, arr.size());
        }
        return arr[index];
    }
    if (type == JTYPE_OBJECT) {
        if (index >= properties.size()) {
            throw json::range_error(line_number, index, properties.size());
        }
        auto itr = properties.at(index);
        if (itr == properties.end()) {
            throw json::parser_error(line_number, "Reached end of properties.");
        }
        return itr->second;
    }
    throw json::parser_error(
        line_number, "Trying to use index-base acces for a " + json::jtype_to_string(type) + " node.");
}

auto jnode_t::operator[](std::size_t index) -> jnode_t &
{
    if (type == JTYPE_ARRAY) {
        if (index >= arr.size()) {
            throw json::range_error(line_number, index, arr.size());
        }
        return arr[index];
    }
    if (type == JTYPE_OBJECT) {
        if (index >= properties.size()) {
            throw json::range_error(line_number, index, properties.size());
        }
        auto itr = properties.at(index);
        if (itr == properties.end()) {
            throw json::parser_error(line_number, "We reached the end of the properties.");
        }
        return itr->second;
    }
    throw json::parser_error(
        line_number, "Trying to use index-base acces for a " + json::jtype_to_string(type) + " node.");
}

auto jnode_t::operator[](const std::string &key) const -> const jnode_t &
{
    if (type == JTYPE_OBJECT) {
        auto itr = properties.find(key);
        if (itr != properties.end()) {
            return itr->second;
        }
    }
    if (json::config::strict_existance_check) {
        throw json::parser_error(
            line_number, "Trying to access the property `" + key + "` for a " + json::jtype_to_string(type) + " node.");
    }
    static jnode_t null_value(JTYPE_NULL);
    return null_value;
}

auto jnode_t::operator[](const std::string &key) -> jnode_t &
{
    if (type == JTYPE_OBJECT) {
        auto itr = properties.find(key);
        if (itr != properties.end()) {
            return itr->second;
        }
        if (json::config::strict_existance_check) {
            throw json::parser_error(line_number, "Trying to access a non-existing property `" + key + "`.");
        }
        return this->add_property(key);
    }
    throw json::parser_error(
        line_number, "Trying to access the property `" + key + "` for a " + json::jtype_to_string(type) + " node.");
}

auto jnode_t::to_string(bool pretty, unsigned tabsize) const -> std::string
{
    return this->to_string_d(1, pretty, tabsize);
}

auto jnode_t::pbegin() const -> jnode_t::property_map_t::const_iterator { return properties.begin(); }

auto jnode_t::pbegin() -> jnode_t::property_map_t::iterator { return properties.begin(); }

auto jnode_t::pend() const -> jnode_t::property_map_t::const_iterator { return properties.end(); }

auto jnode_t::pend() -> jnode_t::property_map_t::iterator { return properties.end(); }

auto jnode_t::abegin() const -> jnode_t::array_data_t::const_iterator { return arr.begin(); }

auto jnode_t::abegin() -> jnode_t::array_data_t::iterator { return arr.begin(); }

auto jnode_t::aend() const -> jnode_t::array_data_t::const_iterator { return arr.end(); }

auto jnode_t::aend() -> jnode_t::array_data_t::iterator { return arr.end(); }

auto jnode_t::to_string_d(unsigned depth, bool pretty, unsigned tabsize) const -> std::string
{
    switch (type) {
    case JTYPE_STRING:
        return this->to_string_d_string();
    case JTYPE_NUMBER:
        return this->to_string_d_number();
    case JTYPE_BOOLEAN:
        return this->to_string_d_boolean();
    case JTYPE_OBJECT:
        return this->to_string_d_object(depth, pretty, tabsize);
    case JTYPE_ARRAY:
        return this->to_string_d_array(depth, pretty, tabsize);
    default:
        return "null";
    }
}

auto jnode_t::to_string_d_string() const -> std::string
{
    std::string string_delimiter(1, config::string_delimiter_character);
    std::string str = value;
    // Replace special characters with escaped equivalents.
    if (json::config::replace_escape_characters) {
        detail::replace_all(str, '\\', "\\\\");
        detail::replace_all(str, '\"', "\\\"");
        detail::replace_all(str, '\t', "\\t");
        detail::replace_all(str, "\r\n", "\\r\\n");
        detail::replace_all(str, '\r', "\\r");
        detail::replace_all(str, '\n', "\\n");
    }
    return string_delimiter + str + string_delimiter;
}

auto jnode_t::to_string_d_number() const -> std::string { return value; }

auto jnode_t::to_string_d_boolean() const -> std::string { return value; }

auto jnode_t::to_string_d_object(unsigned depth, bool pretty, unsigned tabsize) const -> std::string
{
    std::stringstream stream;
    stream << "{";

    if (pretty) {
        stream << "\n";
    }

    auto itr = properties.begin();
    for (; itr != properties.end(); ++itr) {
        if (pretty) {
            stream << detail::make_indentation(depth, tabsize);
        }

        // Serialize the key-value pair.
        stream << config::string_delimiter_character << itr->first << config::string_delimiter_character << ": "
               << itr->second.to_string_d(depth + 1, pretty, tabsize);

        // Add a comma if it's not the last property.
        if (std::next(itr) != properties.end()) {
            stream << ",";
        }

        if (pretty) {
            stream << "\n";
        }
    }

    if (pretty) {
        stream << detail::make_indentation(depth - 1, tabsize);
    }

    stream << "}";
    return stream.str();
}

auto jnode_t::to_string_d_array(unsigned depth, bool pretty, unsigned tabsize) const -> std::string
{
    std::stringstream stream;
    stream << "[";

    for (std::size_t index = 0; index < arr.size(); ++index) {
        if (index != 0) {
            stream << ", ";
        }

        if (pretty && (arr[index].type == JTYPE_ARRAY || arr[index].type == JTYPE_OBJECT)) {
            stream << "\n" << detail::make_indentation(depth, tabsize);
        }

        // Serialize each array element.
        stream << arr[index].to_string_d(depth + 1, pretty, tabsize);
    }

    if (pretty && !arr.empty() && (arr[0].type == JTYPE_ARRAY || arr[0].type == JTYPE_OBJECT)) {
        stream << "\n" << detail::make_indentation(depth - 1, tabsize);
    }

    stream << "]";
    return stream.str();
}

} // namespace json

template <>
auto json::operator<<(json::jnode_t &lhs, const bool &rhs) -> json::jnode_t &
{
    lhs.set_type(json::JTYPE_BOOLEAN);
    lhs.set_value(json::detail::bool_to_string(rhs));
    return lhs;
}

template <>
auto json::operator>>(const json::jnode_t &lhs, bool &rhs) -> const json::jnode_t &
{
    if (json::JTYPE_BOOLEAN == lhs.get_type()) {
        rhs = lhs.as_bool();
    } else if (json::config::strict_type_check) {
        throw json::type_error(lhs.get_line_number(), json::JTYPE_BOOLEAN, lhs.get_type());
    }
    return lhs;
}

template <>
auto json::operator<<(json::jnode_t &lhs, const char &rhs) -> json::jnode_t &
{
    lhs.set_type(json::JTYPE_NUMBER);
    lhs.set_value(json::detail::char_to_string<char>(rhs));
    return lhs;
}

template <>
auto json::operator>>(const json::jnode_t &lhs, char &rhs) -> const json::jnode_t &
{
    if (json::JTYPE_NUMBER == lhs.get_type()) {
        rhs = lhs.as_number<char>();
    } else if (json::config::strict_type_check) {
        throw json::type_error(lhs.get_line_number(), json::JTYPE_NUMBER, lhs.get_type());
    }
    return lhs;
}

template <>
auto json::operator<<(json::jnode_t &lhs, const unsigned char &rhs) -> json::jnode_t &
{
    lhs.set_type(json::JTYPE_NUMBER);
    lhs.set_value(json::detail::char_to_string<unsigned char>(rhs));
    return lhs;
}

template <>
auto json::operator>>(const json::jnode_t &lhs, unsigned char &rhs) -> const json::jnode_t &
{
    if (json::JTYPE_NUMBER == lhs.get_type()) {
        rhs = lhs.as_number<unsigned char>();
    } else if (json::config::strict_type_check) {
        throw json::type_error(lhs.get_line_number(), json::JTYPE_NUMBER, lhs.get_type());
    }
    return lhs;
}

template <>
auto json::operator<<(json::jnode_t &lhs, const short &rhs) -> json::jnode_t &
{
    lhs.set_type(json::JTYPE_NUMBER);
    lhs.set_value(json::detail::number_to_string(rhs));
    return lhs;
}

template <>
auto json::operator>>(const json::jnode_t &lhs, short &rhs) -> const json::jnode_t &
{
    if (json::JTYPE_NUMBER == lhs.get_type()) {
        rhs = lhs.as_number<short>();
    } else if (json::config::strict_type_check) {
        throw json::type_error(lhs.get_line_number(), json::JTYPE_NUMBER, lhs.get_type());
    }
    return lhs;
}

template <>
auto json::operator<<(json::jnode_t &lhs, const unsigned short &rhs) -> json::jnode_t &
{
    lhs.set_type(json::JTYPE_NUMBER);
    lhs.set_value(json::detail::number_to_string(rhs));
    return lhs;
}

template <>
auto json::operator>>(const json::jnode_t &lhs, unsigned short &rhs) -> const json::jnode_t &
{
    if (json::JTYPE_NUMBER == lhs.get_type()) {
        rhs = lhs.as_number<unsigned short>();
    } else if (json::config::strict_type_check) {
        throw json::type_error(lhs.get_line_number(), json::JTYPE_NUMBER, lhs.get_type());
    }
    return lhs;
}

template <>
auto json::operator<<(json::jnode_t &lhs, const int &rhs) -> json::jnode_t &
{
    lhs.set_type(json::JTYPE_NUMBER);
    lhs.set_value(json::detail::number_to_string(rhs));
    return lhs;
}

template <>
auto json::operator>>(const json::jnode_t &lhs, int &rhs) -> const json::jnode_t &
{
    if (json::JTYPE_NUMBER == lhs.get_type()) {
        rhs = lhs.as_number<int>();
    } else if (json::config::strict_type_check) {
        throw json::type_error(lhs.get_line_number(), json::JTYPE_NUMBER, lhs.get_type());
    }
    return lhs;
}

template <>
auto json::operator<<(json::jnode_t &lhs, const unsigned int &rhs) -> json::jnode_t &
{
    lhs.set_type(json::JTYPE_NUMBER);
    lhs.set_value(json::detail::number_to_string(rhs));
    return lhs;
}

template <>
auto json::operator>>(const json::jnode_t &lhs, unsigned int &rhs) -> const json::jnode_t &
{
    if (json::JTYPE_NUMBER == lhs.get_type()) {
        rhs = lhs.as_number<unsigned int>();
    } else if (json::config::strict_type_check) {
        throw json::type_error(lhs.get_line_number(), json::JTYPE_NUMBER, lhs.get_type());
    }
    return lhs;
}

template <>
auto json::operator<<(json::jnode_t &lhs, const long &rhs) -> json::jnode_t &
{
    lhs.set_type(json::JTYPE_NUMBER);
    lhs.set_value(json::detail::number_to_string(rhs));
    return lhs;
}

template <>
auto json::operator>>(const json::jnode_t &lhs, long &rhs) -> const json::jnode_t &
{
    if (json::JTYPE_NUMBER == lhs.get_type()) {
        rhs = lhs.as_number<long>();
    } else if (json::config::strict_type_check) {
        throw json::type_error(lhs.get_line_number(), json::JTYPE_NUMBER, lhs.get_type());
    }
    return lhs;
}

template <>
auto json::operator<<(json::jnode_t &lhs, const unsigned long &rhs) -> json::jnode_t &
{
    lhs.set_type(json::JTYPE_NUMBER);
    lhs.set_value(json::detail::number_to_string(rhs));
    return lhs;
}

template <>
auto json::operator>>(const json::jnode_t &lhs, unsigned long &rhs) -> const json::jnode_t &
{
    if (json::JTYPE_NUMBER == lhs.get_type()) {
        rhs = lhs.as_number<unsigned long>();
    } else if (json::config::strict_type_check) {
        throw json::type_error(lhs.get_line_number(), json::JTYPE_NUMBER, lhs.get_type());
    }
    return lhs;
}

#if __cplusplus >= 201103L

template <>
auto json::operator<<(json::jnode_t &lhs, const long long &rhs) -> json::jnode_t &
{
    lhs.set_type(json::JTYPE_NUMBER);
    lhs.set_value(json::detail::number_to_string(rhs));
    return lhs;
}

template <>
auto json::operator>>(const json::jnode_t &lhs, long long &rhs) -> const json::jnode_t &
{
    if (json::JTYPE_NUMBER == lhs.get_type()) {
        rhs = lhs.as_number<long long>();
    } else if (json::config::strict_type_check) {
        throw json::type_error(lhs.get_line_number(), json::JTYPE_NUMBER, lhs.get_type());
    }
    return lhs;
}

template <>
auto json::operator<<(json::jnode_t &lhs, const unsigned long long &rhs) -> json::jnode_t &
{
    lhs.set_type(json::JTYPE_NUMBER);
    lhs.set_value(json::detail::number_to_string(rhs));
    return lhs;
}

template <>
auto json::operator>>(const json::jnode_t &lhs, unsigned long long &rhs) -> const json::jnode_t &
{
    if (json::JTYPE_NUMBER == lhs.get_type()) {
        rhs = lhs.as_number<unsigned long long>();
    } else if (json::config::strict_type_check) {
        throw json::type_error(lhs.get_line_number(), json::JTYPE_NUMBER, lhs.get_type());
    }
    return lhs;
}

#endif

template <>
auto json::operator<<(json::jnode_t &lhs, const float &rhs) -> json::jnode_t &
{
    lhs.set_type(json::JTYPE_NUMBER);
    lhs.set_value(json::detail::number_to_string(rhs));
    return lhs;
}

template <>
auto json::operator>>(const json::jnode_t &lhs, float &rhs) -> const json::jnode_t &
{
    if (json::JTYPE_NUMBER == lhs.get_type()) {
        rhs = lhs.as_number<float>();
    } else if (json::config::strict_type_check) {
        throw json::type_error(lhs.get_line_number(), json::JTYPE_NUMBER, lhs.get_type());
    }
    return lhs;
}

template <>
auto json::operator<<(json::jnode_t &lhs, const double &rhs) -> json::jnode_t &
{
    lhs.set_type(json::JTYPE_NUMBER);
    lhs.set_value(json::detail::number_to_string(rhs));
    return lhs;
}

template <>
auto json::operator>>(const json::jnode_t &lhs, double &rhs) -> const json::jnode_t &
{
    if (json::JTYPE_NUMBER == lhs.get_type()) {
        rhs = lhs.as_number<double>();
    } else if (json::config::strict_type_check) {
        throw json::type_error(lhs.get_line_number(), json::JTYPE_NUMBER, lhs.get_type());
    }
    return lhs;
}

template <>
auto json::operator<<(json::jnode_t &lhs, const long double &rhs) -> json::jnode_t &
{
    lhs.set_type(json::JTYPE_NUMBER);
    lhs.set_value(json::detail::number_to_string(rhs));
    return lhs;
}

template <>
auto json::operator>>(const json::jnode_t &lhs, long double &rhs) -> const json::jnode_t &
{
    if (json::JTYPE_NUMBER == lhs.get_type()) {
        rhs = lhs.as_number<long double>();
    } else if (json::config::strict_type_check) {
        throw json::type_error(lhs.get_line_number(), json::JTYPE_NUMBER, lhs.get_type());
    }
    return lhs;
}

template <>
auto json::operator<<(json::jnode_t &lhs, const std::string &rhs) -> json::jnode_t &
{
    lhs.set_type(json::JTYPE_STRING);
    lhs.set_value(json::detail::number_to_string(rhs));
    return lhs;
}

template <>
auto json::operator>>(const json::jnode_t &lhs, std::string &rhs) -> const json::jnode_t &
{
    if (json::JTYPE_STRING == lhs.get_type()) {
        rhs = lhs.as_string();
    } else if (json::config::strict_type_check) {
        throw json::type_error(lhs.get_line_number(), json::JTYPE_STRING, lhs.get_type());
    }
    return lhs;
}

auto json::operator<<(std::ostream &lhs, const json::jnode_t &rhs) -> std::ostream &
{
    lhs << rhs.to_string();
    return lhs;
}

auto json::operator<<(std::ofstream &lhs, const json::jnode_t &rhs) -> std::ofstream &
{
    lhs << rhs.to_string();
    return lhs;
}

auto json::operator<<(std::ostream &lhs, const json::detail::token_t &rhs) -> std::ostream &
{
    lhs << rhs.line_number << ", " << json::token_type_to_string(rhs.type) << ", " << "\"" << rhs.value << "\"";
    return lhs;
}

auto json::operator<<(std::ofstream &lhs, const json::detail::token_t &rhs) -> std::ofstream &
{
    lhs << rhs.line_number << ", " << json::token_type_to_string(rhs.type) << ", " << "\"" << rhs.value << "\"";
    return lhs;
}
