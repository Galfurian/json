/// @file json.hpp
/// @author Enrico Fraccaroli (enry.frak@gmail.com)
/// @brief

#include "json/json.hpp"

/// @brief This namespace contains the main json_t class and stream functions.
namespace json
{

/// @brief JSON parser configuration.
namespace config
{
/// @brief If true, the library will throw an error if the C++ variable you
/// are using to read or write to a json node have different types.
bool strict_type_check = false;
/// @brief If true, the library will throw an error if the field of an
/// object you are trying to access does not exist.
bool strict_existance_check = false;
}; // namespace config

/// @brief Transforms the given JSON type to string.
/// @param type the JSON type to transform to string.
/// @return the string representing the JSON type.
std::string jtype_to_string(jtype_t type)
{
    if (type == JSTRING) {
        return "JSTRING";
    }
    if (type == JOBJECT) {
        return "JOBJECT";
    }
    if (type == JARRAY) {
        return "JARRAY";
    }
    if (type == JBOOLEAN) {
        return "JBOOLEAN";
    }
    if (type == JNUMBER) {
        return "JNUMBER";
    }
    if (type == JNULL) {
        return "JNULL";
    }
    if (type == JERROR) {
        return "JERROR";
    }
    return "JUNKNOWN";
}

parser_error::parser_error(std::size_t _line, std::string _message)
    : std::runtime_error(_message),
      line(_line)
{
    // Nothing to do.
}

type_error::type_error(std::size_t _line, json::jtype_t _expected, json::jtype_t _found)
    : json::parser_error(_line, build_message(_expected, _found)),
      expected(_expected),
      found(_found)
{
    // Nothing to do.
}

std::string type_error::build_message(json::jtype_t _expected, json::jtype_t _found)
{
    std::stringstream ss;
    ss << "Trying to access item of type " << json::jtype_to_string(_expected)
       << ", but we found a " << json::jtype_to_string(_found) << ".";
    return ss.str();
}

range_error::range_error(std::size_t _line, std::size_t _index, std::size_t _size)
    : json::parser_error(_line, build_message(_index, _size)),
      index(_index),
      size(_size)
{
    // Nothing to do.
}

std::string range_error::build_message(std::size_t _index, std::size_t _size)
{
    std::stringstream ss;
    ss << "Trying to access item at " << _index << " of " << _size << ".";
    return ss.str();
}

namespace detail
{

/// @brief Access map's elements in a linear fashion.
/// @param map the map we want to access.
/// @param n the index of the element we want to retrieve.
/// @return an interator to the element.
template <class K, class T>
static inline typename std::map<K, T>::const_iterator get_iterator_at(const std::map<K, T> &map, std::size_t n)
{
    typename std::map<K, T>::const_iterator it = map.begin();
    for (std::size_t i = 0; i < n; ++i, ++it) {
        if (it == map.end()) {
            break;
        }
    }
    return it;
}

/// @brief Access map's elements in a linear fashion.
/// @param map the map we want to access.
/// @param n the index of the element we want to retrieve.
/// @return an interator to the element.
template <class K, class T>
static inline typename std::map<K, T>::iterator get_iterator_at(std::map<K, T> &map, std::size_t n)
{
    typename std::map<K, T>::iterator it = map.begin();
    for (std::size_t i = 0; i < n; ++i, ++it) {
        if (it == map.end()) {
            break;
        }
    }
    return it;
}

/// @brief Replaces all the occurences of WHAT with WITH, in INPUT.
/// @param input the string we want to manipulate.
/// @param what the string we want to replace.
/// @param with the string we use as replacement.
/// @return a reference to the input string.
std::string &replace_all(std::string &input, const std::string &what, const std::string &with)
{
    std::size_t i = 0;
    while ((i = input.find(what, i)) != std::string::npos) {
        input.replace(i, what.size(), with);
        i += with.size();
    }
    return input;
}

/// @brief Replaces all the occurences of WHAT with WITH, in INPUT.
/// @param input the string we want to manipulate.
/// @param what the char we want to replace.
/// @param with the string we use as replacement.
/// @return a reference to the input string.
std::string &replace_all(std::string &input, char what, const std::string &with)
{
    std::size_t i = 0;
    while ((i = input.find(what, i)) != std::string::npos) {
        input.replace(i, 1U, with);
        i += with.size();
    }
    return input;
}

/// @brief Transforms the boolean value to string.
/// @param value the boolean value.
/// @return the string representation of the boolean value.
static inline std::string bool_to_string(bool value)
{
    return value ? "true" : "false";
}

/// @brief Transforms the ASCII integer representing the character into a string.
/// @param value the input character.
/// @return the output string.
template <typename T>
static inline std::string char_to_string(T value)
{
    std::stringstream ss;
    ss << static_cast<int>(value);
    return ss.str();
}

/// @brief Transforms the number to string.
/// @param value the input number.
/// @return the output string.
template <typename T>
static inline std::string number_to_string(T value)
{
    std::stringstream ss;
    ss << value;
    return ss.str();
}

/// @brief Generates the indenation.
/// @param depth depth of the indentation.
/// @param tabsize the number of character for each depth unit.
/// @return the output indentation as string.
static inline std::string make_indentation(unsigned depth, unsigned tabsize = 4)
{
    return std::string(depth * tabsize, ' ');
}

/// @brief Checks if the given character is a newline.
/// @param c the input character.
/// @return if the character is a newline.
bool isnewline(char c)
{
    return (c == '\n') || (c == '\r');
}

/// @brief Searches for the next whitespace in SOURCE starting from I.
/// @param source the source string.
/// @param index the index.
/// @return the index of the next whitespace.
std::size_t next_whitespace(const std::string &source, std::size_t index)
{
    std::size_t slength = source.length();
    while (index < slength) {
        if (source[index] == '"') {
            ++index;
            while (index < slength) {
                if (source[index] == '"') {
                    if (source[index - 1] != '\\') {
                        break;
                    }
                    if ((source[index - 2] == '\\') && (source[index - 3] != '\\')) {
                        break;
                    }
                }
                ++index;
            }
        }
        if (source[index] == '\'') {
            ++index;
            while (index < slength) {
                if (source[index] == '\'') {
                    if (source[index - 1] != '\\') {
                        break;
                    }
                    if (source[index - 2] == '\\') {
                        break;
                    }
                }
                ++index;
            }
        }
        if (std::isspace(source[index])) {
            return index;
        }
        ++index;
    }
    return slength;
}

/// @brief Skips the whitespaces starting from INDEX.
/// @param source the sources string.
/// @param index the index we start skipping from.
/// @param line_number the current line number.
/// @return the index of the next non-whitespace character.
std::size_t skip_whitespaces(const std::string &source, std::size_t index, std::size_t &line_number)
{
    std::size_t slength = source.length();
    while (index < slength) {
        if (!std::isspace(source[index])) {
            return index;
        }
        line_number += detail::isnewline(source[index]);
        ++index;
    }
    return slength;
}

/// @brief Deserializes the given string.
/// @param ref the input string.
/// @return the deserialized input string.
std::string deserialize(const std::string &ref)
{
    std::size_t i, j, offset;
    std::string out;
    for (i = 0; i < ref.length(); ++i) {
        if ((ref[i] == '\\') && ((i + 1) < ref.length())) {
            offset = 2;
            if (ref[i + 1] == '\"') {
                out.push_back('"');
            } else if (ref[i + 1] == '\\') {
                out.push_back('\\');
            } else if (ref[i + 1] == '/') {
                out.push_back('/');
            } else if (ref[i + 1] == 'b') {
                out.push_back('\b');
            } else if (ref[i + 1] == 'f') {
                out.push_back('\f');
            } else if (ref[i + 1] == 'n') {
                out.push_back('\n');
            } else if (ref[i + 1] == 'r') {
                out.push_back('\r');
            } else if (ref[i + 1] == 't') {
                out.push_back('\t');
            } else if (ref[i + 1] == 'u' && i + 5 < ref.length()) {
                int value = 0;
                for (j = 0; j < 4; j++) {
                    value *= 16;
                    if (ref[i + 2 + j] <= '9' && ref[i + 2 + j] >= '0') {
                        value += ref[i + 2 + j] - '0';
                    }
                    if (ref[i + 2 + j] <= 'f' && ref[i + 2 + j] >= 'a') {
                        value += ref[i + 2 + j] - 'a' + 10;
                    }
                }
                out.push_back(static_cast<char>(value));
                offset = 6;
            }
            i += offset - 1;
            continue;
        }
        out.push_back(ref[i]);
    }
    return out;
}

std::vector<token_t> &tokenize(const std::string &source, std::vector<token_t> &tokens)
{
    std::size_t line_number = 0;
    std::size_t index = 0, next = 0;
    index = detail::skip_whitespaces(source, 0, line_number);
    while (index <= source.size()) {
        next = detail::next_whitespace(source, index);
        if (next == index) {
            break;
        }
        std::string str = source.substr(index, next - index);
        std::size_t k   = 0;
        while (k < str.length()) {
            if (str[k] == '"') {
                std::size_t j = k + 1;
                while (j < str.length()) {
                    if (str[j] == '"') {
                        if (str[j - 1] != '\\') {
                            break;
                        }
                        if ((str[j - 2] == '\\') && (str[j - 3] != '\\')) {
                            break;
                        }
                    }
                    ++j;
                }
                tokens.push_back(token_t(str.substr(k + 1, j - k - 1), STRING, line_number));
                k = j + 1;
                continue;
            }
            if (str[k] == '\'') {
                std::size_t j = k + 1;
                while (j < str.length()) {
                    if (str[j] == '\'') {
                        if (str[j - 1] != '\\') {
                            break;
                        }
                        if (str[j - 2] == '\\') {
                            break;
                        }
                    }
                    ++j;
                }
                tokens.push_back(token_t(str.substr(k + 1, j - k - 1), STRING, line_number));
                k = j + 1;
                continue;
            }
            if (str[k] == ',') {
                tokens.push_back(token_t(",", COMMA, line_number));
                ++k;
                continue;
            }
            if (str[k] == 't' && k + 3 < str.length() && str.substr(k, 4) == "true") {
                tokens.push_back(token_t("true", BOOLEAN, line_number));
                k += 4;
                continue;
            }
            if (str[k] == 'f' && k + 4 < str.length() && str.substr(k, 5) == "false") {
                tokens.push_back(token_t("false", BOOLEAN, line_number));
                k += 5;
                continue;
            }
            if (str[k] == 'n' && k + 3 < str.length() && str.substr(k, 4) == "null") {
                tokens.push_back(token_t("null", NUL, line_number));
                k += 4;
                continue;
            }
            if (str[k] == '}') {
                tokens.push_back(token_t("}", CURLY_CLOSE, line_number));
                ++k;
                continue;
            }
            if (str[k] == '{') {
                tokens.push_back(token_t("{", CURLY_OPEN, line_number));
                ++k;
                continue;
            }
            if (str[k] == ']') {
                tokens.push_back(token_t("]", BRACKET_CLOSE, line_number));
                ++k;
                continue;
            }
            if (str[k] == '[') {
                tokens.push_back(token_t("[", BRACKET_OPEN, line_number));
                ++k;
                continue;
            }
            if (str[k] == ':') {
                tokens.push_back(token_t(":", COLON, line_number));
                ++k;
                continue;
            }
            if (str[k] == '-' || std::isdigit(str[k])) {
                std::size_t k2 = k;
                if (str[k2] == '-') {
                    ++k2;
                }
                while (k2 < str.size()) {
                    if ((str[k2] != '.') && !std::isdigit(str[k2])) {
                        if ((str[k2] != 'e') && (str[k2] != 'E')) {
                            break;
                        }
                        if ((k2 + 1) >= str.size()) {
                            break;
                        }
                        if ((str[k2 + 1] != '+') && (str[k2 + 1] != '-')) {
                            if (std::isdigit(str[k2 + 1])) {
                                k2 += 2;
                            } else {
                                break;
                            }
                        } else {
                            if ((k2 + 3) >= str.size()) {
                                break;
                            }
                            if (std::isdigit(str[k2 + 3])) {
                                k2 += 3;
                            } else {
                                break;
                            }
                        }
                    }
                    ++k2;
                }
                tokens.push_back(token_t(str.substr(k, k2 - k), NUMBER, line_number));
                k = k2;
                continue;
            }
            tokens.push_back(token_t(str.substr(k), UNKNOWN, line_number));
            k = str.length();
        }
        index = detail::skip_whitespaces(source, next, line_number);
    }
    return tokens;
}

jnode_t &json_parse(std::vector<token_t> &tokens, std::size_t index, std::size_t &output_index, jnode_t &current)
{
    // Let us keep track a previous index.
    std::size_t next_index;
    // A pointer to the key.
    const char *key;
    //
    // Set line number.
    current.set_line_number(tokens[index].line_number + 1);
    // Parse the element.
    if (tokens[index].type == CURLY_OPEN) {
        // We need to skip the braket, and check if we ran out of tokens.
        if ((++index) >= tokens.size()) {
            throw json::parser_error(current.get_line_number(), "We ran out of tokens.");
        }
        // Set type.
        current.set_type(JOBJECT);
        // Iterate until we find the end of the object, i.e., the closing braket.
        while (tokens[index].type != CURLY_CLOSE) {
            // Set the key.
            key = tokens[index].value.c_str();
            // We need to skip the key, and check if we ran out of tokens.
            if ((++index) >= tokens.size()) {
                throw json::parser_error(current.get_line_number(), "We ran out of tokens.");
            }
            // We should find a COLON ':'.
            if (tokens[index].type != COLON) {
                throw json::parser_error(current.get_line_number(), "We did not find a COLON.");
            }
            // We need to skip the COLON, and check if we ran out of tokens.
            if ((++index) >= tokens.size()) {
                throw json::parser_error(current.get_line_number(), "We ran out of tokens.");
            }
            // Set the next_index.
            next_index = index;
            // Add the property.
            jnode_t &property = current.add_property(key);
            // Build the property.
            json_parse(tokens, index, next_index, property);
            // Update the index.
            index = next_index;
            // If the next token is a comma, we need to parse another property,
            // but we also need to skip that comma.
            index += (tokens[index].type == COMMA);
            // Now, if the index goes outside the number of tokens we need to stop.
            if (index >= tokens.size()) {
                throw json::parser_error(current.get_line_number(), "We ran out of tokens.");
            }
        }
    } else if (tokens[index].type == BRACKET_OPEN) {
        // We need to skip the braket, and check if we ran out of tokens.
        if ((++index) >= tokens.size()) {
            throw json::parser_error(current.get_line_number(), "We ran out of tokens.");
        }
        // Set type.
        current.set_type(JARRAY);
        // Iterate until we find the end of the array, i.e., the closing braket.
        while (tokens[index].type != BRACKET_CLOSE) {
            // Set the next_index.
            next_index = index;
            // Add the element.
            jnode_t &element = current.add_element();
            // Build the element.
            json_parse(tokens, index, next_index, element);
            // Update the index.
            index = next_index;
            // If the next token is a comma, we need to parse another property,
            // but we also need to skip that comma.
            index += (tokens[index].type == COMMA);
            // Now, if the index goes outside the number of tokens we need to stop.
            if (index >= tokens.size()) {
                throw json::parser_error(current.get_line_number(), "We ran out of tokens.");
            }
        }
    } else if (tokens[index].type == NUMBER) {
        // Set type.
        current.set_type(JNUMBER);
        // Set the value.
        current.set_value(tokens[index].value);
    } else if (tokens[index].type == STRING) {
        // Set type.
        current.set_type(JSTRING);
        // Set the value.
        current.set_value(tokens[index].value);
    } else if (tokens[index].type == BOOLEAN) {
        // Set type.
        current.set_type(JBOOLEAN);
        // Set the value.
        current.set_value(tokens[index].value);
    } else if (tokens[index].type == NUL) {
        // Set type.
        current.set_type(JNULL);
        // Set the value.
        current.set_value("null");
    } else {
        throw json::parser_error(current.get_line_number(), "Cannot type the entry.");
    }
    // Move to the next token.
    output_index = index + 1;
    return current;
}

} // namespace detail

/// @brief Contains parsing functions.
namespace parser
{

/// @brief Parse the json formatted string.
/// @param json_string The json formatted string.
/// @return the root of the generated json tree.
jnode_t parse(const std::string &json_string)
{
    std::size_t k = 0;
    std::vector<detail::token_t> tokens;
    // Extract the tokens.
    detail::tokenize(json_string, tokens);
    // Prepare the root.
    jnode_t root;
    // Parse the tokens.
    return detail::json_parse(tokens, 0UL, k, root);
}

/// @brief Parse the json file.
/// @param filename Path to the json file.
/// @return the root of the generated json tree.
bool read_file(const std::string &filename, std::string &content)
{
    std::ifstream in(filename.c_str());
    if (!in.is_open()) {
        return false;
    }
    std::stringstream ss;
    ss << in.rdbuf() << " ";
    content.append(ss.str());
    in.close();
    return true;
}

/// @brief Parse the json file.
/// @param filename Path to the json file.
/// @return the root of the generated json tree.
jnode_t parse_file(const std::string &filename)
{
    std::string content;
    if (!json::parser::read_file(filename, content)) {
        jnode_t null_value(JNULL);
        return null_value;
    }
    return parser::parse(content);
}

/// @brief Write the json node on file.
/// @param filename Path to the json file.
/// @param node     The json tree to write.
/// @param pretty   Enable/Disable pretty print of json.
/// @param tabsize	The dimension of tabulation (if pretto == true).
/// @return if the operation is a success.
bool write_file(const std::string &filename, const jnode_t &node, bool pretty, unsigned tabsize)
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
    : type(JNULL),
      value(),
      line_number(),
      properties(),
      arr()
{
    // Nothing to do.
}

jnode_t::jnode_t(jtype_t _type)
    : type(_type),
      value(),
      line_number(),
      properties(),
      arr()
{
    // Nothing to do.
}

std::string jnode_t::get_value() const
{
    return value;
}

jtype_t jnode_t::get_type() const
{
    return type;
}

bool jnode_t::is_string() const
{
    return type == JSTRING;
}

bool jnode_t::is_bool() const
{
    return type == JBOOLEAN;
}

bool jnode_t::is_array() const
{
    return type == JARRAY;
}

bool jnode_t::is_object() const
{
    return type == JOBJECT;
}

bool jnode_t::is_number() const
{
    return type == JNUMBER;
}

bool jnode_t::is_null() const
{
    return type == JNULL;
}

std::size_t jnode_t::get_line_number() const
{
    return line_number;
}

std::size_t jnode_t::size() const
{
    if (type == JARRAY) {
        return arr.size();
    }
    if (type == JOBJECT) {
        return properties.size();
    }
    return 0;
}

bool jnode_t::has_property(const std::string &key) const
{
    if (type == JOBJECT) {
        return properties.find(key) != properties.end();
    }
    return false;
}

bool jnode_t::as_bool() const
{
    if (type == JBOOLEAN) {
        return value == "true";
    }
    if (json::config::strict_type_check) {
        throw json::type_error(line_number, JBOOLEAN, type);
    }
    return false;
}

std::string jnode_t::as_string() const
{
    if (type == JSTRING) {
        return detail::deserialize(value);
    }
    if (json::config::strict_type_check) {
        throw json::type_error(line_number, JSTRING, type);
    }
    return std::string();
}

jnode_t &jnode_t::set_type(jtype_t _type)
{
    type = _type;
    return *this;
}

jnode_t &jnode_t::set_value(const std::string &_value)
{
    if ((type != JOBJECT) && (type != JARRAY)) {
        value = _value;
    } else {
        throw json::parser_error(line_number, "Trying to set the value of a " + json::jtype_to_string(type) + " node.");
    }
    return *this;
}

jnode_t &jnode_t::set_line_number(std::size_t _line_number)
{
    line_number = _line_number;
    return *this;
}

jnode_t &jnode_t::add_property(const std::string &key)
{
    if (type != JOBJECT) {
        throw json::parser_error(line_number, "Trying to add a property to a " + json::jtype_to_string(type) + " node.");
    }
    return properties.set(key, jnode_t())->second;
}

jnode_t &jnode_t::add_property(const std::string &key, const jnode_t &node)
{
    if (type != JOBJECT) {
        throw json::parser_error(line_number, "Trying to add a property to a " + json::jtype_to_string(type) + " node.");
    }
    return properties.set(key, node)->second;
}

void jnode_t::remove_property(const std::string &key)
{
    if (type != JOBJECT) {
        throw json::parser_error(line_number, "Trying to remove a property from a " + json::jtype_to_string(type) + " node.");
    }
    properties.erase(key);
}

jnode_t &jnode_t::add_element(const jnode_t &node)
{
    if (type != JARRAY) {
        throw json::parser_error(line_number, "Trying to add an element to a " + json::jtype_to_string(type) + " node.");
    }
    arr.push_back(node);
    return arr.back();
}

void jnode_t::remove_element(std::size_t index)
{
    if (type != JARRAY) {
        throw json::parser_error(line_number, "Trying to add an element to a " + json::jtype_to_string(type) + " node.");
    }
    if (index >= arr.size()) {
        throw json::range_error(line_number, index, arr.size());
    }
    arr.erase(arr.begin() + static_cast<std::ptrdiff_t>(index));
}

void jnode_t::reserve(std::size_t size)
{
    if (type != JARRAY) {
        throw json::parser_error(line_number, "Trying to reserve space in a " + json::jtype_to_string(type) + " node.");
    }
    arr.reserve(size);
}

void jnode_t::resize(std::size_t size)
{
    if (type != JARRAY) {
        throw json::parser_error(line_number, "Trying to reserve space in a " + json::jtype_to_string(type) + " node.");
    }
    arr.resize(size);
}

void jnode_t::clear()
{
    // We set the type to error, so that the node must be properly set again to
    // make the tree coherent again.
    type = JERROR;
    value.clear();
    properties.clear();
    arr.clear();
}

const jnode_t &jnode_t::operator[](std::size_t i) const
{
    if (type == JARRAY) {
        if (i >= arr.size()) {
            throw json::range_error(line_number, i, arr.size());
        }
        return arr[i];
    }
    if (type == JOBJECT) {
        if (i >= properties.size()) {
            throw json::range_error(line_number, i, properties.size());
        }
        property_map_t::const_iterator it = properties.at(i);
        if (it == properties.end()) {
            throw json::parser_error(line_number, "Reached end of properties.");
        }
        return it->second;
    }
    throw json::parser_error(line_number, "Trying to use index-base acces for a " + json::jtype_to_string(type) + " node.");
}

jnode_t &jnode_t::operator[](std::size_t i)
{
    if (type == JARRAY) {
        if (i >= arr.size()) {
            throw json::range_error(line_number, i, arr.size());
        }
        return arr[i];
    }
    if (type == JOBJECT) {
        if (i >= properties.size()) {
            throw json::range_error(line_number, i, properties.size());
        }
        property_map_t::iterator it = properties.at(i);
        if (it == properties.end()) {
            throw json::parser_error(line_number, "We reached the end of the properties.");
        }
        return it->second;
    }
    throw json::parser_error(line_number, "Trying to use index-base acces for a " + json::jtype_to_string(type) + " node.");
}

const jnode_t &jnode_t::operator[](const std::string &key) const
{
    if (type == JOBJECT) {
        property_map_t::const_iterator it = properties.find(key);
        if (it != properties.end()) {
            return it->second;
        }
    }
    if (json::config::strict_existance_check) {
        throw json::parser_error(line_number, "Trying to access the property `" + key + "` for a " + json::jtype_to_string(type) + " node.");
    }
    static jnode_t null_value(JNULL);
    return null_value;
}

jnode_t &jnode_t::operator[](const std::string &key)
{
    if (type == JOBJECT) {
        property_map_t::iterator it = properties.find(key);
        if (it != properties.end()) {
            return it->second;
        }
        if (json::config::strict_existance_check) {
            throw json::parser_error(line_number, "Trying to access a non-existing property `" + key + "`.");
        }
        return this->add_property(key);
    }
    throw json::parser_error(line_number, "Trying to access the property `" + key + "` for a " + json::jtype_to_string(type) + " node.");
}

std::string jnode_t::to_string(bool pretty, unsigned tabsize) const
{
    return this->to_string_d(1, pretty, tabsize);
}

jnode_t::property_map_t::const_iterator jnode_t::pbegin() const
{
    return properties.begin();
}

jnode_t::property_map_t::iterator jnode_t::pbegin()
{
    return properties.begin();
}

jnode_t::property_map_t::const_iterator jnode_t::pend() const
{
    return properties.end();
}

jnode_t::property_map_t::iterator jnode_t::pend()
{
    return properties.end();
}

jnode_t::array_data_t::const_iterator jnode_t::abegin() const
{
    return arr.begin();
}

jnode_t::array_data_t::iterator jnode_t::abegin()
{
    return arr.begin();
}

jnode_t::array_data_t::const_iterator jnode_t::aend() const
{
    return arr.end();
}

jnode_t::array_data_t::iterator jnode_t::aend()
{
    return arr.end();
}

std::string jnode_t::to_string_d(unsigned depth, bool pretty, unsigned tabsize) const
{
    std::stringstream ss;
    if (type == JSTRING) {
        return std::string("\"") + value + std::string("\"");
    }
    if (type == JNUMBER) {
        return value;
    }
    if (type == JBOOLEAN) {
        return value;
    }
    if (type == JOBJECT) {
        ss << "{";
        if (pretty) {
            ss << "\n";
        }
        property_map_t::const_iterator it = properties.begin();
        for (it = properties.begin(); it != properties.end(); ++it) {
            if (pretty) {
                ss << detail::make_indentation(depth, tabsize);
            }
            ss << "\"" << it->first << "\": "
               << it->second.to_string_d(depth + 1, pretty, tabsize)
               << ((std::distance(it, properties.end()) == 1) ? "" : ",");
            if (pretty) {
                ss << "\n";
            }
        }
        if (pretty) {
            ss << detail::make_indentation(depth - 1, tabsize);
        }
        ss << "}";
        return ss.str();
    }
    if (type == JARRAY) {
        ss << "[";
        for (std::size_t i = 0; i < arr.size(); ++i) {
            if (i) {
                ss << ", ";
            }
            if (pretty && ((arr[i].type == JARRAY) || (arr[i].type == JOBJECT))) {
                ss << "\n"
                   << detail::make_indentation(depth, tabsize);
            }
            ss << arr[i].to_string_d(depth + 1, pretty, tabsize);
        }
        if (pretty && !arr.empty() && ((arr[0].type == JARRAY) || (arr[0].type == JOBJECT))) {
            ss << "\n"
               << detail::make_indentation(depth - 1, tabsize);
        }
        ss << "]";
        return ss.str();
    }
    return "null";
}

/// @brief Allows to easily implement stream operators.
#define JSON_DEFINE_OP(json_type, type, write_function, read_function)                \
    template <>                                                                       \
    jnode_t &operator<<(jnode_t &lhs, const type &rhs)                                \
    {                                                                                 \
        lhs.set_type(json_type);                                                      \
        lhs.set_value(write_function(rhs));                                           \
        return lhs;                                                                   \
    }                                                                                 \
    template <>                                                                       \
    const jnode_t &operator>>(const jnode_t &lhs, type &rhs)                          \
    {                                                                                 \
        if ((json_type) == lhs.get_type()) {                                          \
            rhs = static_cast<type>(lhs.read_function());                             \
        } else if (json::config::strict_type_check) {                                 \
            throw json::type_error(lhs.get_line_number(), json_type, lhs.get_type()); \
        }                                                                             \
        return lhs;                                                                   \
    }

JSON_DEFINE_OP(json::JBOOLEAN, bool, json::detail::bool_to_string, as_bool)
JSON_DEFINE_OP(json::JNUMBER, char, json::detail::char_to_string<char>, as_number<int>)
JSON_DEFINE_OP(json::JNUMBER, unsigned char, json::detail::char_to_string<unsigned char>, as_number<unsigned int>)
JSON_DEFINE_OP(json::JNUMBER, short, json::detail::number_to_string, as_number<short>)
JSON_DEFINE_OP(json::JNUMBER, unsigned short, json::detail::number_to_string, as_number<unsigned short>)
JSON_DEFINE_OP(json::JNUMBER, int, json::detail::number_to_string, as_number<int>)
JSON_DEFINE_OP(json::JNUMBER, unsigned int, json::detail::number_to_string, as_number<unsigned int>)
JSON_DEFINE_OP(json::JNUMBER, long, json::detail::number_to_string, as_number<long>)
JSON_DEFINE_OP(json::JNUMBER, unsigned long, json::detail::number_to_string, as_number<unsigned long>)
JSON_DEFINE_OP(json::JNUMBER, float, json::detail::number_to_string, as_number<float>)
JSON_DEFINE_OP(json::JNUMBER, double, json::detail::number_to_string, as_number<double>)
JSON_DEFINE_OP(json::JNUMBER, long double, json::detail::number_to_string, as_number<long double>)
JSON_DEFINE_OP(json::JSTRING, std::string, json::detail::number_to_string, as_string)
#if __cplusplus >= 201103L
JSON_DEFINE_OP(json::JNUMBER, long long, json::detail::number_to_string, as_number<long long>)
JSON_DEFINE_OP(json::JNUMBER, unsigned long long, json::detail::number_to_string, as_number<unsigned long long>)
#endif

#undef JSON_DEFINE_OP

} // namespace json

/// @brief Sends the JSON node to the output stream.
/// @param lhs the stream we are writing the content of the JSON node.
/// @param rhs the JSON node.
/// @return a reference to the output stream.
std::ostream &operator<<(std::ostream &lhs, const json::jnode_t &rhs)
{
    lhs << rhs.to_string();
    return lhs;
}

/// @brief Sends the JSON node to the output file stream.
/// @param lhs the stream we are writing the content of the JSON node.
/// @param rhs the JSON node.
/// @return a reference to the output file stream.
std::ofstream &operator<<(std::ofstream &lhs, const json::jnode_t &rhs)
{
    lhs << rhs.to_string();
    return lhs;
}
