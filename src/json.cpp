/// @file json.cpp
/// @author Enrico Fraccaroli (enry.frak@gmail.com)
/// @brief Implement the functionality of the jnode_t class.
///
/// @copyright (c) 2024 This file is distributed under the MIT License.
/// See LICENSE.md for details.
///

#include "json/json.hpp"

#include <cstdint>
#include <regex>

/// @brief This namespace contains the main json_t class and stream functions.
namespace json
{

namespace config
{
bool strict_type_check          = false;
bool strict_existance_check     = false;
bool replace_escape_characters  = false;
char string_delimiter_character = '\'';
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

/// @brief Access map's elements in a linear fashion.
/// @param map the map we want to access.
/// @param n the index of the element we want to retrieve.
/// @return an interator to the element.
template <class K, class T>
static inline auto get_iterator_at(const std::map<K, T> &map, std::size_t n) -> typename std::map<K, T>::const_iterator
{
    typename std::map<K, T>::const_iterator itr = map.begin();
    for (std::size_t index = 0; index < n; ++index, ++itr) {
        if (itr == map.end()) {
            break;
        }
    }
    return itr;
}

/// @brief Access map's elements in a linear fashion.
/// @param map the map we want to access.
/// @param n the index of the element we want to retrieve.
/// @return an interator to the element.
template <class K, class T>
static inline auto get_iterator_at(std::map<K, T> &map, std::size_t n) -> typename std::map<K, T>::iterator
{
    typename std::map<K, T>::iterator itr = map.begin();
    for (std::size_t index = 0; index < n; ++index, ++itr) {
        if (itr == map.end()) {
            break;
        }
    }
    return itr;
}

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

/// @brief Checks if the given character is a newline.
/// @param c the input character.
/// @return if the character is a newline.
static inline auto isnewline(char chr) -> bool { return (chr == '\n') || (chr == '\r'); }

/// @brief Skips a quoted string in SOURCE starting from INDEX.
/// @param source The source string.
/// @param index The current index.
/// @return The index after the closing quote.
static auto skip_quoted_string(const std::string &source, std::size_t index) -> std::size_t
{
    // Store the opening quote (' or ") and move to the next character
    char quote = source[index++];
    // Scan the string.
    for (; index < source.length(); ++index) {
        // Check if we've reached the closing quote.
        if (source[index] == quote) {
            // If the quote is not escaped, return the index after the closing
            // quote.
            if (source[index - 1] != '\\') {
                return index + 1;
            }
            // Handle cases where an escaped quote might still be valid.
            if (index > 1 && source[index - 2] == '\\' && source[index - 3] != '\\') {
                return index + 1;
            }
        }
    }
    // Return the current position if no closing quote is found.
    return index;
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
/// @return The index after the closing "*/".
static auto skip_multi_line_comment(const std::string &source, std::size_t index) -> std::size_t
{
    // Scan the string.
    for (std::size_t slength = source.length(); index < slength; ++index) {
        // Check if we've reached the closing comment.
        if ((source[index] == '*') && (index + 1 < slength) && (source[index + 1] == '/')) {
            return index + 2;
        }
    }
    // Return the current position if no closing comment is found.
    return index;
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
        if (std::isspace(source[index]) == 0) {
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
        } else if (std::isspace(source[index]) > 0) {
            return index;
        } else {
            ++index;
        }
    }
    return slength;
}

/// @brief Converts a hexadecimal Unicode escape sequence to a character.
/// @param source The input string.
/// @param index The index where the escape sequence starts.
/// @return The converted character.
static auto extract_unicode_escape(const std::string &source, std::size_t index) -> char
{
    int value = 0;
    for (std::size_t j = 0; j < 4; j++) {
        value *= 16;
        if (source[index + j] >= '0' && source[index + j] <= '9') {
            value += source[index + j] - '0';
        } else if (source[index + j] >= 'a' && source[index + j] <= 'f') {
            value += source[index + j] - 'a' + 10;
        }
    }
    return static_cast<char>(value);
}

/// @brief Decodes an escape sequence.
/// @param source The input string.
/// @param index The current index.
/// @param offset Reference to an offset variable to update.
/// @return The decoded character.
static auto extract_escape_sequence(const std::string &source, std::size_t index, std::size_t &offset) -> char
{
    offset = 2;
    switch (source[index + 1]) {
    case '\"':
    case '\'':
        return config::string_delimiter_character;
    case '\\':
        return '\\';
    case '/':
        return '/';
    case 'b':
        return '\b';
    case 'f':
        return '\f';
    case 'n':
        return '\n';
    case 'r':
        return '\r';
    case 't':
        return '\t';
    case 'u':
        if (index + 5 < source.length()) {
            offset = 6;
            return extract_unicode_escape(source, index + 2);
        }
        break;
    default:
        break;
    }
    // Fallback for unexpected escape sequences.
    return source[index + 1];
}

/// @brief Extracts a number token.
/// @param source The input string.
/// @param index The starting index.
/// @return The index after the number.
static auto extract_number(const std::string &source, std::size_t index) -> std::size_t
{
    std::size_t slength = source.length();
    // Skip the sign.
    if (source[index] == '-') {
        ++index;
    }
    // Skip the integer part.
    while (index < slength) {
        if ((std::isdigit(source[index]) != 0) || (source[index] == '.')) {
            ++index;
        } else if ((source[index] == 'e' || source[index] == 'E') && index + 1 < slength) {
            if ((source[index + 1] == '+') || (source[index + 1] == '-') || (std::isdigit(source[index + 1]) != 0)) {
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
/// @return The deserialized input string.
static inline auto deserialize(const std::string &source) -> std::string
{
    std::size_t offset = 0;
    std::string out;

    for (std::size_t index = 0; index < source.length(); ++index) {
        if (source[index] == '\\' && (index + 1) < source.length()) {
            out.push_back(extract_escape_sequence(source, index, offset));
            index += offset - 1;
        } else {
            out.push_back(source[index]);
        }
    }
    return out;
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
            // Add the comment to the tokens.
            tokens.emplace_back(source.substr(index, end - index), JTOKEN_COMMENT, line_number);
            // Skip the comment.
            index = end;
        } else if ((source[index] == '"') || (source[index] == '\'')) {
            // Find the end of the quoted string.
            std::size_t end = skip_quoted_string(source, index);
            // Add the string to the tokens.
            tokens.emplace_back(source.substr(index + 1, end - index - 2), JTOKEN_STRING, line_number);
            // Skip the string.
            index = end;
        } else if (source[index] == ',') {
            // Add the comma to the tokens.
            tokens.emplace_back(",", JTOKEN_COMMA, line_number);
            // Skip the comma.
            ++index;
        } else if (source[index] == 't' && index + 3 < slength && source.substr(index, 4) == "true") {
            // Add the boolean to the tokens.
            tokens.emplace_back("true", JTOKEN_BOOLEAN, line_number);
            // Skip the boolean.
            index += 4;
        } else if (source[index] == 'f' && index + 4 < slength && source.substr(index, 5) == "false") {
            // Add the boolean to the tokens.
            tokens.emplace_back("false", JTOKEN_BOOLEAN, line_number);
            // Skip the boolean.
            index += 5;
        } else if (source[index] == 'n' && index + 3 < slength && source.substr(index, 4) == "null") {
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
        } else if (source[index] == '-' || (std::isdigit(source[index]) != 0)) {
            // Extract the number.
            std::size_t end = extract_number(source, index);
            // Add the number to the tokens.
            tokens.emplace_back(source.substr(index, end - index), JTOKEN_NUMBER, line_number);
            // Update the index.
            index = end;
        } else {
            // Extract the string until the next colon.
            std::size_t end = source.find(':', index);
            // If we did not find a colon, we extract the whole string.
            if (end == std::string::npos) {
                end = slength;
            }
            // Add the string to the tokens.
            tokens.emplace_back(source.substr(index, end - index), JTOKEN_STRING, line_number);
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
/// @details Iterates through the token list starting from the current index,
/// skipping any tokens of type `JTOKEN_COMMENT`. Throws an error if the index
/// exceeds the bounds of the token list.
///
/// @param tokens The list of tokens to parse.
/// @param index The current index in the token list, which will be updated.
auto skip_comments(const std::vector<token_t> &tokens, std::size_t &index) -> void
{
    while (index < tokens.size()) {
        // Check if index is out of bounds.
        if (index >= tokens.size()) {
            throw json::parser_error(0, "Error: Index out of bounds while skipping comments.");
        }
        // Break the loop if the current token is not a comment.
        if (tokens[index].type != JTOKEN_COMMENT) {
            break;
        }
        // Move to the next token.
        ++index;
    }
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
auto json_parse(std::vector<token_t> &tokens, std::size_t index, std::size_t &output_index, jnode_t &current)
    -> jnode_t &
{
    // Track the next index for recursive parsing.
    std::size_t next_index = 0;
    // Pointer to the key in key-value pairs.
    const char *key        = nullptr;
    // Set the line number for error reporting.
    current.set_line_number(tokens[index].line_number + 1);
    // Skip any comment tokens before parsing.
    skip_comments(tokens, index);
    // Parse JSON object if the token is an opening curly brace.
    if (tokens[index].type == JTOKEN_CURLY_OPEN) {
        // Skip the opening curly brace.
        skip_tokens(tokens, index, 1, current);
        // Set the current node type to object.
        current.set_type(JTYPE_OBJECT);
        // Parse key-value pairs until the closing curly brace is found.
        while (tokens[index].type != JTOKEN_CURLY_CLOSE) {
            // Skip comments before the key.
            skip_comments(tokens, index);
            // Set the key from the current token value.
            key = tokens[index].value.c_str();
            // Skip the key token.
            skip_tokens(tokens, index, 1, current);
            // Ensure the next token is a colon separating the key and value.
            if (tokens[index].type != JTOKEN_COLON) {
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
            json_parse(tokens, index, next_index, property);
            // Update the index after parsing the value.
            index = next_index;
            // Skip any comments after the value.
            skip_comments(tokens, index);
            // If a comma is found, skip it to parse the next key-value pair.
            skip_tokens(tokens, index, static_cast<std::size_t>(tokens[index].type == JTOKEN_COMMA), current);
            // Skip any comments after the comma.
            skip_comments(tokens, index);
        }
    }
    // Parse JSON array if the token is an opening bracket.
    else if (tokens[index].type == JTOKEN_BRACKET_OPEN) {
        // Skip the opening bracket.
        skip_tokens(tokens, index, 1, current);
        // Set the current node type to array.
        current.set_type(JTYPE_ARRAY);
        // Parse array elements until the closing bracket is found.
        while (tokens[index].type != JTOKEN_BRACKET_CLOSE) {
            // Skip comments before the element.
            skip_comments(tokens, index);
            // Track the index before parsing the element.
            next_index       = index;
            // Add a new element to the current array.
            jnode_t &element = current.add_element();
            // Recursively parse the array element.
            json_parse(tokens, index, next_index, element);
            // Update the index after parsing the element.
            index = next_index;
            // If a comma is found, skip it to parse the next element.
            skip_tokens(tokens, index, static_cast<std::size_t>(tokens[index].type == JTOKEN_COMMA), current);
        }
    }
    // Parse JSON primitive values (number, string, boolean, null).
    else if (tokens[index].type == JTOKEN_NUMBER) {
        current.set_type(JTYPE_NUMBER);
        current.set_value(tokens[index].value);
    } else if (tokens[index].type == JTOKEN_STRING) {
        current.set_type(JTYPE_STRING);
        // Replace escaped newlines in the string value.
        current.set_value(std::regex_replace(tokens[index].value, std::regex("\\\\[ \t]*\n"), "\n"));
    } else if (tokens[index].type == JTOKEN_BOOLEAN) {
        current.set_type(JTYPE_BOOLEAN);
        current.set_value(tokens[index].value);
    } else if (tokens[index].type == JTOKEN_NULL) {
        current.set_type(JTYPE_NULL);
        current.set_value("null");
    } else {
        // Throw an error if the token type is unrecognized.
        throw json::parser_error(current.get_line_number(), "Cannot type the entry.");
    }
    // Skip any comments after parsing the element.
    skip_comments(tokens, index);
    // Move to the next token and update the output index.
    output_index = index + 1;
    return current;
}

} // namespace detail

/// @brief Contains parsing functions.
namespace parser
{

auto parse(const std::string &json_string) -> jnode_t
{
    std::size_t index = 0;
    std::vector<detail::token_t> tokens;
    // Extract the tokens.
    detail::tokenize(json_string, tokens);
    // Prepare the root.
    jnode_t root;
    // Parse the tokens.
    return detail::json_parse(tokens, 0UL, index, root);
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

auto parse_file(const std::string &filename) -> jnode_t
{
    std::string content;
    if (!json::parser::read_file(filename, content)) {
        jnode_t null_value(JTYPE_NULL);
        return null_value;
    }
    return parser::parse(content);
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
