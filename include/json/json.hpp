/// @file json.hpp
/// @author Enrico Fraccaroli (enry.frak@gmail.com)
/// @brief

#pragma once

#include <algorithm>

#include <iostream>
#include <cassert>
#include <sstream>
#include <fstream>
#include <cstring>
#include <string>
#include <vector>
#include <list>
#include <set>
#include <map>

#ifdef __cpp_lib_span
#include <span>
#endif

namespace json
{

/// @brief JSON types.
enum jtype_t {
    JSTRING,
    JOBJECT,
    JARRAY,
    JBOOLEAN,
    JNUMBER,
    JNULL,
    JUNKNOWN,
    JERROR
};

/// @brief Represent a json node.
class jnode_t {
public:
    /// Map that allows to easily access a child node based on the name.
    typedef std::map<std::string, jnode_t> property_map_t;
    /// Sorting function.
    typedef bool (*sort_function_t)(const jnode_t &, const jnode_t &);

    /// @brief Constructor.
    jnode_t();

    /// @brief Constructor.
    /// @param _type The type to set.
    explicit jnode_t(jtype_t _type);

    /// @brief Returns the value of the json node.
    std::string get_value() const;

    /// @brief Returns the type of the json node.
    jtype_t get_type() const;

    /// @brief Checks wheter the node is a JSTRING.
    inline bool is_string() const;

    /// @brief Checks wheter the node is a JBOOLEAN.
    inline bool is_bool() const;

    /// @brief Checks wheter the node is a JARRAY.
    inline bool is_array() const;

    /// @brief Checks wheter the node is a JOBJECT.
    inline bool is_object() const;

    /// @brief Checks wheter the node is a JNUMBER.
    inline bool is_number() const;

    /// @brief Checks wheter the node is a JNULL.
    inline bool is_null() const;

    /// @brief Returns the original line number.
    int get_line_number() const;

    /// @brief Returns the size of the internal array.
    size_t size() const;

    /// @brief Checks if the current object has the given property.
    /// @param key The key of the property.
    /// @return If the object has the property.
    bool has_property(const std::string &key) const;

    /// @brief Turns the value to INT.
    /// @param check_type If true, check the correspondence of types.
    /// @return The extracted value
    int as_int() const;

    /// @brief Turns the value to DOUBLE.
    /// @param check_type If true, check the correspondence of types.
    /// @return The extracted value
    double as_double() const;

    /// @brief Turns the value to BOOL.
    /// @param check_type If true, check the correspondence of types.
    /// @return The extracted value
    bool as_bool() const;

    /// @brief Turns the value to STRING.
    /// @param check_type If true, check the correspondence of types.
    /// @return The extracted value
    std::string as_string() const;

    /// @brief Sets the type.
    /// @param _type The type to set.
    jnode_t &set_type(jtype_t _type);

    /// @brief Sets the internal value.
    /// @param _value The value to set.
    jnode_t &set_value(const std::string &_value);

    /// @brief Sets the line number.
    /// @param _line_number The line number to set.
    jnode_t &set_line_number(int _line_number);

    /// @brief Adds a new property with the given key.
    /// @param key The key of the property.
    /// @return A reference to the newly created property.
    jnode_t &add_property(const std::string &key);

    /// @brief Remove the property with the given key.
    /// @param key The key of the property.
    void remove_property(const std::string &key);

    /// @brief Ads a given property with the given key.
    /// @param key  The key of the property.
    /// @param node The property to set.
    /// @return A reference to the newly created property.
    jnode_t &add_property(const std::string &key, const jnode_t &node);

    /// @brief Adds the element to the array.
    /// @param node The node to add.
    jnode_t &add_element(const jnode_t &node);

    /// @brief Reserves the given space for the array.
    void reserve(size_t size);

    /// @brief Resize the given space for the array.
    void resize(size_t size);

    /// @brief Clears all the internal data structures.
    void clear();

    /// @brief Allows to sort the entry inside the node, which be a JARRAY.
    /// @param sort_function The function used to sort.
    void sort(sort_function_t sort_function);

    /// @brief Provides access to an internal node.
    /// @param i The index of the node.
    /// @return A const reference to the node. If this node is not
    ///          an array, or an object, returns a node of type JNULL.
    const jnode_t &operator[](size_t i) const;

    /// @brief Provides access to an internal node (THIS=Object/Array).
    /// @param i The index of the node.
    /// @return A reference to the node. If this node is not an array,
    ///          or an object, returns a node of type JNULL.
    jnode_t &operator[](size_t i);

    /// @brief Provides access to an internal node.
    /// @param key The key of the internal node.
    /// @return A const reference to the node. If this node is not
    ///          an array, or an object, returns a node of type JNULL.
    const jnode_t &operator[](const std::string &key) const;

    /// @brief Provides access to an internal node.
    /// @param key The key of the internal node.
    /// @return A reference to the node. If this node is not
    ///          an array, or an object, returns a node of type JNULL.
    jnode_t &operator[](const std::string &key);

    /// @brief Turns the json object to a formatted string.
    /// @param pretty   Enable/Disable pretty print of json.
    /// @param tabsize	The dimension of tabulation (if pretto == true).
    std::string to_string(bool pretty = true, unsigned tabsize = 4) const;

    /// @brief Returns a constant iterator pointing to the **beginning** of the property map.
    /// @return the iterator.
    property_map_t::const_iterator begin() const;

    /// @brief Returns an iterator pointing to the **beginning** of the property map.
    /// @return the iterator.
    property_map_t::iterator begin();

    /// @brief Returns a constant iterator pointing to the **end** of the property map.
    /// @return the iterator.
    property_map_t::const_iterator end() const;

    /// @brief Returns an iterator pointing to the **end** of the property map.
    /// @return the iterator.
    property_map_t::iterator end();

private:
    /// @brief Turns the json object to a formatted string.
    /// @param depth    The current depth, used for indentation (if pretty == true).
    /// @param pretty   Enable/Disable pretty print of json.
    /// @param tabsize	The dimension of tabulation (if pretty == true).
    std::string to_string_d(int depth, bool pretty = true, unsigned tabsize = 4) const;

    /// The type of the node.
    jtype_t type;
    /// The value contained inside the node.
    std::string value;
    /// The original line number.
    int line_number;
    /// The properties of the node.
    property_map_t properties;
    /// The array content.
    std::vector<jnode_t> arr;
};

namespace detail
{

inline std::string jtype_to_string(jtype_t type)
{
    if (type == JSTRING)
        return "JSTRING";
    if (type == JOBJECT)
        return "JOBJECT";
    if (type == JARRAY)
        return "JARRAY";
    if (type == JBOOLEAN)
        return "JBOOLEAN";
    if (type == JNUMBER)
        return "JNUMBER";
    if (type == JNULL)
        return "JNULL";
    if (type == JERROR)
        return "JERROR";
    return "JUNKNOWN";
}

template <class K, class T>
inline typename std::map<K, T>::const_iterator get_iterator_at(const std::map<K, T> &map, size_t n)
{
    typename std::map<K, T>::const_iterator it = map.begin();
    for (size_t i = 0; i < n; ++i, ++it)
        assert(it != map.end());
    return it;
}

template <class K, class T>
inline typename std::map<K, T>::iterator get_iterator_at(std::map<K, T> &map, size_t n)
{
    typename std::map<K, T>::iterator it = map.begin();
    for (size_t i = 0; i < n; ++i, ++it)
        assert(it != map.end());
    return it;
}

inline std::string &replace_all(std::string &s, const std::string &what, const std::string &with)
{
    size_t i = 0;
    while ((i = s.find(what, i)) != std::string::npos) {
        s.replace(i, what.size(), with);
        i += with.size();
    }
    return s;
}

inline std::string &replace_all(std::string &s, char what, const std::string &with)
{
    size_t i = 0;
    while ((i = s.find(what, i)) != std::string::npos) {
        s.replace(i, 1U, with);
        i += with.size();
    }
    return s;
}

inline std::string bool_to_string(bool value)
{
    return value ? "true" : "false";
}

inline std::string char_to_string(char value)
{
    std::stringstream ss;
    ss << static_cast<int>(value);
    return ss.str();
}

template <typename T>
inline std::string value_to_string(T value)
{
    std::stringstream ss;
    ss << value;
    return ss.str();
}

inline std::string make_indentation(unsigned depth, unsigned tabsize = 4)
{
    return std::string(depth * tabsize, ' ');
}

inline bool is_whitespace(char c)
{
    return isspace(c);
}

inline bool is_number(char c)
{
    return (c >= '0') && (c <= '9');
}

inline bool is_newline(char c)
{
    return (c == '\n') || (c == '\r');
}

inline int next_whitespace(const std::string &source, int i)
{
    int slength = static_cast<int>(source.length());
    while (i < slength) {
        if (source[i] == '"') {
            ++i;
            while ((i < slength) && (source[i] != '"' || source[i - 1] == '\\')) {
                ++i;
            }
        }
        if (source[i] == '\'') {
            ++i;
            while ((i < slength) && (source[i] != '\'' || source[i - 1] == '\\')) {
                ++i;
            }
        }
        if (is_whitespace(source[i])) {
            return i;
        }
        ++i;
    }
    return slength;
}

inline int skip_whitespaces(const std::string &source, int i, int &line_number)
{
    while (i < static_cast<int>(source.length())) {
        if (!is_whitespace(source[i])) {
            return i;
        }
        line_number += is_newline(source[i]);
        ++i;
    }
    return -1;
}

inline std::string deserialize(const std::string &ref)
{
    std::string out;
    for (size_t i = 0; i < ref.length(); ++i) {
        if ((ref[i] == '\\') && ((i + 1) < ref.length())) {
            int plus = 2;
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
                unsigned long v = 0;
                for (int j = 0; j < 4; j++) {
                    v *= 16;
                    if (ref[i + 2 + j] <= '9' && ref[i + 2 + j] >= '0')
                        v += ref[i + 2 + j] - '0';
                    if (ref[i + 2 + j] <= 'f' && ref[i + 2 + j] >= 'a')
                        v += ref[i + 2 + j] - 'a' + 10;
                }
                out.push_back((char)v);
                plus = 6;
            }
            i += plus - 1;
            continue;
        }
        out.push_back(ref[i]);
    }
    return out;
}

enum token_type_t {
    UNKNOWN,
    STRING,
    NUMBER,
    CURLY_OPEN,
    CURLY_CLOSE,
    BRACKET_OPEN,
    BRACKET_CLOSE,
    COMMA,
    COLON,
    BOOLEAN,
    NUL
};

typedef struct token_t {
    /// The value.
    std::string value;
    /// The type.
    token_type_t type;
    /// The line number.
    int line_number;

    /// @brief Constructor.
    /// @param value
    /// @param type
    explicit token_t(const std::string &_value = "", token_type_t _type = UNKNOWN, int _line_number = 0)
        : value(_value),
          type(_type),
          line_number(_line_number)
    {
        // Nothing to do.
    }
} token_t;

std::vector<token_t> &tokenize(const std::string &source, std::vector<token_t> &tokens)
{
    int line_number = 0;
    int index       = detail::skip_whitespaces(source, 0, line_number);
    while (index >= 0) {
        int next        = detail::next_whitespace(source, index);
        std::string str = source.substr(index, next - index);
        size_t k        = 0;
        while (k < str.length()) {
            if (str[k] == '"') {
                size_t tmp_k = k + 1;
                while (tmp_k < str.length() && (str[tmp_k] != '"' || str[tmp_k - 1] == '\\')) {
                    ++tmp_k;
                }
                tokens.push_back(token_t(str.substr(k + 1, tmp_k - k - 1), STRING, line_number));
                k = tmp_k + 1;
                continue;
            }
            if (str[k] == '\'') {
                size_t tmp_k = k + 1;
                while (tmp_k < str.length() && (str[tmp_k] != '\'' || str[tmp_k - 1] == '\\')) {
                    ++tmp_k;
                }
                tokens.push_back(token_t(str.substr(k + 1, tmp_k - k - 1), STRING, line_number));
                k = tmp_k + 1;
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
            if (str[k] == '-' || detail::is_number(str[k])) {
                size_t k2 = k;
                if (str[k2] == '-') {
                    ++k2;
                }
                while (k2 < str.size()) {
                    if ((str[k2] != '.') && !detail::is_number(str[k2])) {
                        if ((str[k2] != 'e') && (str[k2] != 'E')) {
                            break;
                        }
                        if ((k2 + 1) >= str.size()) {
                            break;
                        }
                        if ((str[k2 + 1] != '+') && (str[k2 + 1] != '-')) {
                            if (detail::is_number(str[k2 + 1])) {
                                k2 += 2;
                            } else {
                                break;
                            }
                        } else {
                            if ((k2 + 3) >= str.size()) {
                                break;
                            }
                            if (detail::is_number(str[k2 + 3])) {
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
            if (str[k] == '#' && (k + 1 < str.length()) && str.substr(k, 2) == "##") {
                tokens.push_back(token_t(str.substr(k, 2), UNKNOWN, line_number));
                k += 2;
                continue;
            }
            tokens.push_back(token_t(str.substr(k), UNKNOWN, line_number));
            k = str.length();
        }
        index = detail::skip_whitespaces(source, next, line_number);
    }
    return tokens;
}

jnode_t json_parse(std::vector<token_t> &v, int i, int &r)
{
    jnode_t current;
    // Set line number.
    current.set_line_number(v[i].line_number + 1);
    // Parse the element.
    if (v[i].type == CURLY_OPEN) {
        // Set type.
        current.set_type(JOBJECT);
        // Set the value.
        ++i;
        while (v[i].type != CURLY_CLOSE) {
            std::string key = v[i].value;
            i += 2; // k+1 should be ':'
            int j        = i;
            current[key] = json_parse(v, i, j);
            i            = j;
            if (v[i].type == COMMA) {
                ++i;
            }
        }
    } else if (v[i].type == BRACKET_OPEN) {
        // Set type.
        current.set_type(JARRAY);
        // Set the value.
        ++i;
        while (v[i].type != BRACKET_CLOSE) {
            int j = i;
            current.add_element(json_parse(v, i, j));
            i = j;
            if (v[i].type == COMMA) {
                ++i;
            }
        }
    } else if (v[i].type == NUMBER) {
        // Set type.
        current.set_type(JNUMBER);
        // Set the value.
        current.set_value(v[i].value);
    } else if (v[i].type == STRING) {
        // Set type.
        current.set_type(JSTRING);
        // Replace protected special characters, with the actual ones.
        detail::replace_all(v[i].value, "\\n", "\n");
        detail::replace_all(v[i].value, "\\\"", "\"");
        // Set the value.
        current.set_value(v[i].value);
    } else if (v[i].type == BOOLEAN) {
        // Set type.
        current.set_type(JBOOLEAN);
        // Set the value.
        current.set_value(v[i].value);
    } else if (v[i].type == NUL) {
        // Set type.
        current.set_type(JNULL);
        // Set the value.
        current.set_value("null");
    } else {
        // Set type.
        current.set_type(JUNKNOWN);
        // Set the value.
        current.set_value("##");
    }
    // Move to the next token.
    r = i + 1;
    return current;
}

} // namespace detail

namespace parser
{

/// @brief Parse the json formatted string.
/// @param json_string The json formatted string.
/// @return the root of the generated json tree.
jnode_t parse(const std::string &json_string)
{
    int k = 0;
    std::vector<detail::token_t> tokens;
    return detail::json_parse(detail::tokenize(json_string, tokens), 0, k);
}

/// @brief Parse the json file.
/// @param filename Path to the json file.
/// @return the root of the generated json tree.
jnode_t parse_file(const std::string &filename)
{
    std::ifstream in(filename.c_str());
    if (!in.is_open()) {
        jnode_t null_value(JNULL);
        return null_value;
    }
    std::stringstream ss;
    ss << in.rdbuf() << " ";
    in.close();
    return parser::parse(ss.str());
}

/// @brief Write the json node on file.
/// @param filename Path to the json file.
/// @param node     The json tree to write.
/// @param pretty   Enable/Disable pretty print of json.
/// @param tabsize	The dimension of tabulation (if pretto == true).
/// @return if the operation is a success.
bool write_file(const std::string &filename, const jnode_t &node, bool pretty = true, unsigned tabsize = 4)
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

/// @brief Represents an out-of-bound error.
class RangeError : public std::out_of_range {
public:
    /// @brief Construct a new range error.
    RangeError(size_t index, size_t size)
        : std::out_of_range("Trying to access item at " + detail::value_to_string(index) + " of " + detail::value_to_string(size) + ".")
    {
        // Nothing to do.
    }
};

/// @brief Represents a type error.
class TypeError : public std::runtime_error {
public:
    /// @brief Construct a new type error.
    TypeError(size_t line, jtype_t expected, jtype_t found)
        : std::runtime_error("line " + detail::value_to_string(line) + " : Expecting " + detail::jtype_to_string(expected) + " but found " + detail::jtype_to_string(found) + ".")
    {
        // Nothing to do.
    }
};

jnode_t::jnode_t()
    : type(JUNKNOWN),
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

inline std::string jnode_t::get_value() const
{
    return value;
}

inline jtype_t jnode_t::get_type() const
{
    return type;
}

inline bool jnode_t::is_string() const
{
    return type == JSTRING;
}

inline bool jnode_t::is_bool() const
{
    return type == JBOOLEAN;
}

inline bool jnode_t::is_array() const
{
    return type == JARRAY;
}

inline bool jnode_t::is_object() const
{
    return type == JOBJECT;
}

inline bool jnode_t::is_number() const
{
    return type == JNUMBER;
}

inline bool jnode_t::is_null() const
{
    return type == JNULL;
}

inline int jnode_t::get_line_number() const
{
    return line_number;
}

inline size_t jnode_t::size() const
{
    if (type == JARRAY)
        return arr.size();
    if (type == JOBJECT)
        return properties.size();
    return 0;
}

inline bool jnode_t::has_property(const std::string &key) const
{
    if (type == JOBJECT)
        return properties.find(key) != properties.end();
    return false;
}

inline int jnode_t::as_int() const
{
    if (type != JNUMBER) {
#ifdef STRICT_TYPE_CHECK
        throw TypeError(line_number, JNUMBER, type);
#else
        return 0;
#endif
    }
    std::stringstream ss;
    ss << value;
    int k;
    ss >> k;
    return k;
}

inline double jnode_t::as_double() const
{
    if (type != JNUMBER) {
#ifdef STRICT_TYPE_CHECK
        throw TypeError(line_number, JNUMBER, type);
#else
        return .0;
#endif
    }
    std::stringstream ss;
    ss << value;
    double k;
    ss >> k;
    return k;
}

inline bool jnode_t::as_bool() const
{
    if (type != JBOOLEAN) {
#ifdef STRICT_TYPE_CHECK
        throw TypeError(line_number, JBOOLEAN, type);
#else
        return false;
#endif
    }
    return value == "true";
}

inline std::string jnode_t::as_string() const
{
    if (type != JSTRING) {
#ifdef STRICT_TYPE_CHECK
        throw TypeError(line_number, JSTRING, type);
#else
        return std::string();
#endif
    }
    return detail::deserialize(value);
}

inline jnode_t &jnode_t::set_type(jtype_t _type)
{
    type = _type;
    return *this;
}

inline jnode_t &jnode_t::set_value(const std::string &_value)
{
    value = _value;
    return *this;
}

inline jnode_t &jnode_t::set_line_number(int _line_number)
{
    line_number = _line_number;
    return *this;
}

inline jnode_t &jnode_t::add_property(const std::string &key)
{
    return (properties[key] = jnode_t());
}

inline void jnode_t::remove_property(const std::string &key)
{
    property_map_t::iterator it = properties.find(key);
    if (it != properties.end()) {
        properties.erase(it);
    }
}

inline jnode_t &jnode_t::add_property(const std::string &key, const jnode_t &node)
{
    return (properties[key] = node);
}

inline jnode_t &jnode_t::add_element(const jnode_t &node)
{
    arr.push_back(node);
    return arr.back();
}

inline void jnode_t::reserve(size_t size)
{
    arr.reserve(size);
}

inline void jnode_t::resize(size_t size)
{
    arr.resize(size);
}

inline void jnode_t::clear()
{
    value.clear();
    type = JUNKNOWN;
    properties.clear();
    arr.clear();
}

inline void jnode_t::sort(sort_function_t sort_function)
{
    if (type == JARRAY)
        std::sort(arr.begin(), arr.end(), sort_function);
}

inline const jnode_t &jnode_t::operator[](size_t i) const
{
    if (type == JARRAY) {
        if (i >= arr.size())
            throw RangeError(i, arr.size());
        return arr[i];
    } else if (type == JOBJECT) {
        if (i >= properties.size())
            throw RangeError(i, properties.size());
        return detail::get_iterator_at(properties, i)->second;
    }
    static jnode_t null_value(JNULL);
    return null_value;
}

inline jnode_t &jnode_t::operator[](size_t i)
{
    if (type == JARRAY) {
        if (i >= arr.size())
            throw RangeError(i, arr.size());
        return arr[i];
    } else if (type == JOBJECT) {
        if (i >= properties.size())
            throw RangeError(i, properties.size());
        return detail::get_iterator_at(properties, i)->second;
    }
    static jnode_t null_value(JNULL);
    return null_value;
}

inline const jnode_t &jnode_t::operator[](const std::string &key) const
{
    if (type == JOBJECT) {
        property_map_t::const_iterator it = properties.find(key);
        if (it != properties.end()) {
            return it->second;
        }
    }
    static jnode_t null_value(JNULL);
    return null_value;
}

inline jnode_t &jnode_t::operator[](const std::string &key)
{
    if (type == JOBJECT) {
        property_map_t::iterator it = properties.find(key);
        if (it != properties.end()) {
            return it->second;
        }
        return this->add_property(key);
    }
    static jnode_t null_value(JNULL);
    return null_value;
}

inline std::string jnode_t::to_string(bool pretty, unsigned tabsize) const
{
    return this->to_string_d(1, pretty, tabsize);
}

inline jnode_t::property_map_t::const_iterator jnode_t::begin() const
{
    return properties.begin();
}

/// @brief Returns an iterator pointing to the **beginning** of the property map.
/// @return the iterator.
inline jnode_t::property_map_t::iterator jnode_t::begin()
{
    return properties.begin();
}

inline jnode_t::property_map_t::const_iterator jnode_t::end() const
{
    return properties.end();
}

inline jnode_t::property_map_t::iterator jnode_t::end()
{
    return properties.end();
}

std::string jnode_t::to_string_d(int depth, bool pretty, unsigned tabsize) const
{
    std::stringstream ss;
    if (type == JSTRING) {
        std::string str = value;
        // Replace special characters, with UTF-8 supported ones.
        detail::replace_all(str, '\\', "\\\\");
        detail::replace_all(str, '\"', "\\\"");
        detail::replace_all(str, '\t', "\\t");
        detail::replace_all(str, "\r\n", "\\r\\n");
        detail::replace_all(str, '\r', "\\r");
        detail::replace_all(str, '\n', "\\n");
        return std::string("\"") + str + std::string("\"");
    }
    if (type == JNUMBER)
        return value;
    if (type == JBOOLEAN)
        return value;
    if (type == JNULL)
        return "null";
    if (type == JOBJECT) {
        ss << "{";
        if (pretty)
            ss << "\n";
        property_map_t::const_iterator it = properties.begin();
        for (it = properties.begin(); it != properties.end(); ++it) {
            if (pretty)
                ss << detail::make_indentation(depth, tabsize);
            ss << "\"" << it->first << "\": "
               << it->second.to_string_d(depth + 1, pretty, tabsize)
               << ((std::distance(it, properties.end()) == 1) ? "" : ",");
            if (pretty)
                ss << "\n";
        }
        if (pretty)
            ss << detail::make_indentation(depth - 1, tabsize);
        ss << "}";
        return ss.str();
    }
    if (type == JARRAY) {
        ss << "[";
        for (size_t i = 0; i < arr.size(); i++) {
            if (i)
                ss << ", ";
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
    return "##";
}

// == From C++ to JSON ========================================================
template <typename T>
json::jnode_t &operator<<(json::jnode_t &lhs, T const &rhs);

template <typename T>
json::jnode_t &operator<<(json::jnode_t &lhs, T *const &rhs)
{
    return lhs << (*rhs);
}

inline json::jnode_t &operator<<(json::jnode_t &lhs, char const *rhs)
{
    lhs.set_type(json::JSTRING);
    lhs.set_value(rhs);
    return lhs;
}

inline json::jnode_t &operator<<(json::jnode_t &lhs, char *rhs)
{
    lhs.set_type(json::JSTRING);
    lhs.set_value(rhs);
    return lhs;
}

template <typename T>
json::jnode_t &operator<<(json::jnode_t &lhs, std::vector<T> const &rhs)
{
    lhs.clear();
    lhs.set_type(json::JARRAY);
    lhs.resize(rhs.size());
    for (size_t i = 0; i < rhs.size(); ++i) {
        lhs[i] << rhs[i];
    }
    return lhs;
}

#ifdef __cpp_lib_span
template <typename T>
json::jnode_t &operator<<(json::jnode_t &lhs, std::span<T> rhs)
{
    lhs.clear();
    lhs.set_type(json::JARRAY);
    lhs.resize(rhs.size());
    for (size_t i = 0; i < rhs.size(); ++i) {
        lhs[i] << rhs[i];
    }
    return lhs;
}
#endif

template <typename T>
json::jnode_t &operator<<(json::jnode_t &lhs, std::list<T> const &rhs)
{
    lhs.clear();
    lhs.set_type(json::JARRAY);
    lhs.resize(rhs.size());
    size_t i = 0;
    for (typename std::list<T>::const_iterator it = rhs.begin(); it != rhs.end(); ++it) {
        lhs[i++] << (*it);
    }
    return lhs;
}

template <typename T>
json::jnode_t &operator<<(json::jnode_t &lhs, std::set<T> const &rhs)
{
    lhs.clear();
    lhs.set_type(json::JARRAY);
    lhs.resize(rhs.size());
    size_t i = 0;
    for (typename std::set<T>::const_iterator it = rhs.begin(); it != rhs.end(); ++it) {
        lhs[i++] << (*it);
    }
    return lhs;
}

template <typename T1, typename T2>
json::jnode_t &operator<<(json::jnode_t &lhs, std::map<T1, T2> const &rhs)
{
    lhs.clear();
    lhs.set_type(json::JOBJECT);
    typename std::map<T1, T2>::const_iterator it;
    for (it = rhs.begin(); it != rhs.end(); ++it) {
        std::stringstream ss;
        ss << it->first;
        lhs.add_property(ss.str()) << it->second;
    }
    return lhs;
}

// == From JSON to C++ ========================================================
template <typename T>
const json::jnode_t &operator>>(const json::jnode_t &lhs, T &rhs);

template <typename T>
const json::jnode_t &operator>>(const json::jnode_t &lhs, T *&rhs)
{
    return lhs >> (*rhs);
}

template <typename T>
const json::jnode_t &operator>>(const json::jnode_t &lhs, std::vector<T> &rhs)
{
    if (lhs.get_type() == json::JARRAY) {
        // Clear the vector.
        rhs.clear();
        // Resize the vector.
        rhs.resize(lhs.size());
        // Load its elements.
        for (size_t i = 0; i < lhs.size(); ++i) {
            lhs[i] >> rhs[i];
        }
    }
    return lhs;
}

#ifdef __cpp_lib_span
// NOTE: span must have sufficient size to fit all elements of json array
template <typename T>
const json::jnode_t &operator>>(const json::jnode_t &lhs, std::span<T> rhs)
{
    if (lhs.get_type() == json::JARRAY) {
        assert(lhs.size() <= rhs.size());
        // NOTE: This should not be necessary (see assert above) but for safety reasons, ensure there is no out of bounds acces
        const std::size_t elem_count = lhs.size() < rhs.size() ? lhs.size() : rhs.size();
        for (size_t i = 0; i < elem_count; ++i) {
            lhs[i] >> rhs[i];
        }
    }
    return lhs;
}
#endif

template <typename T>
const json::jnode_t &operator>>(const json::jnode_t &lhs, std::list<T> &rhs)
{
    if (lhs.get_type() == json::JARRAY) {
        rhs.clear();
        rhs.resize(lhs.size());
        size_t i = 0;
        for (typename std::list<T>::iterator it = rhs.begin(); it != rhs.end(); ++it) {
            lhs[i++] >> (*it);
        }
    }
    return lhs;
}

template <typename T>
const json::jnode_t &operator>>(const json::jnode_t &lhs, std::set<T> &rhs)
{
    if (lhs.get_type() == json::JARRAY) {
        rhs.clear();
        for (size_t i = 0; i < lhs.size(); ++i) {
            T t;
            lhs[i] >> t;
            rhs.insert(t);
        }
    }
    return lhs;
}

template <typename T1, typename T2>
const json::jnode_t &operator>>(const json::jnode_t &lhs, std::map<T1, T2> &rhs)
{
    // Check the type.
    if (lhs.get_type() == json::JOBJECT) {
        rhs.clear();
        json::jnode_t::property_map_t::const_iterator it;
        for (it = lhs.begin(); it != lhs.end(); ++it) {
            std::stringstream ss;
            ss << it->first;
            T1 key;
            ss >> key;
            it->second >> rhs[key];
        }
    }
    return lhs;
}

#define JSON_DEFINE_OP(json_type, type, write_function, read_function)          \
    template <>                                                                 \
    inline json::jnode_t &operator<<(json::jnode_t &lhs, const type &rhs)       \
    {                                                                           \
        lhs.set_type(json_type);                                                \
        lhs.set_value(write_function(rhs));                                     \
        return lhs;                                                             \
    }                                                                           \
    template <>                                                                 \
    inline const json::jnode_t &operator>>(const json::jnode_t &lhs, type &rhs) \
    {                                                                           \
        rhs = static_cast<type>(lhs.read_function());                           \
        return lhs;                                                             \
    }

JSON_DEFINE_OP(json::JBOOLEAN, bool, json::detail::bool_to_string, as_bool)
JSON_DEFINE_OP(json::JNUMBER, char, json::detail::char_to_string, as_int)
JSON_DEFINE_OP(json::JNUMBER, unsigned char, json::detail::char_to_string, as_int)
JSON_DEFINE_OP(json::JNUMBER, short, json::detail::value_to_string, as_int)
JSON_DEFINE_OP(json::JNUMBER, unsigned short, json::detail::value_to_string, as_int)
JSON_DEFINE_OP(json::JNUMBER, int, json::detail::value_to_string, as_int)
JSON_DEFINE_OP(json::JNUMBER, unsigned int, json::detail::value_to_string, as_int)
JSON_DEFINE_OP(json::JNUMBER, long, json::detail::value_to_string, as_int)
JSON_DEFINE_OP(json::JNUMBER, unsigned long, json::detail::value_to_string, as_int)
JSON_DEFINE_OP(json::JNUMBER, long long, json::detail::value_to_string, as_int)
JSON_DEFINE_OP(json::JNUMBER, unsigned long long, json::detail::value_to_string, as_int)
JSON_DEFINE_OP(json::JNUMBER, float, json::detail::value_to_string, as_double)
JSON_DEFINE_OP(json::JNUMBER, double, json::detail::value_to_string, as_double)
JSON_DEFINE_OP(json::JNUMBER, long double, json::detail::value_to_string, as_double)
JSON_DEFINE_OP(json::JSTRING, std::string, json::detail::value_to_string, as_string)

#undef JSON_DEFINE_OP

} // namespace json

/// @brief Define the pair of operators required to handle C++ enums.
#define JSON_DEFINE_OP_ENUM(type)                                        \
    template <>                                                          \
    json::jnode_t &operator<<(json::jnode_t &lhs, const type &rhs)       \
    {                                                                    \
        return (lhs << static_cast<int>(rhs));                           \
    }                                                                    \
    template <>                                                          \
    const json::jnode_t &operator>>(const json::jnode_t &lhs, type &rhs) \
    {                                                                    \
        rhs = static_cast<type>(lhs.as_int());                           \
        return lhs;                                                      \
    }

inline std::ostream &operator<<(std::ostream &lhs, const json::jnode_t &rhs)
{
    lhs << rhs.to_string();
    return lhs;
}

inline std::ofstream &operator<<(std::ofstream &lhs, const json::jnode_t &rhs)
{
    lhs << rhs.to_string();
    return lhs;
}
