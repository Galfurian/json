/// @file json.hpp
/// @author Enrico Fraccaroli (enry.frak@gmail.com)
/// @brief

#pragma once

#include <algorithm>
#include <cassert>
#include <cstring>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#ifdef __cpp_lib_span
#include <span>
#endif

#include "ordered_map/ordered_map.hpp"

/// @brief This namespace contains the main json_t class and stream functions.
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
    JERROR
};

/// @brief Transforms the given JSON type to string.
/// @param type the JSON type to transform to string.
/// @return the string representing the JSON type.
std::string jtype_to_string(jtype_t type);

/// @brief Represents a type error.
class parser_error : public std::runtime_error {
public:
    const std::size_t line;

    /// @brief Construct a new type error.
    /// @param line the line where the error was found.
    /// @param message the error message.
    parser_error(std::size_t _line, std::string _message);
};

/// @brief Represents a type error.
class type_error : public json::parser_error {
public:
    const json::jtype_t expected;
    const json::jtype_t found;

    /// @brief Construct a new type error.
    /// @param line the line where the error was found.
    /// @param message the error message.
    type_error(std::size_t _line, json::jtype_t _expected, json::jtype_t _found);

private:
    static std::string build_message(json::jtype_t _expected, json::jtype_t _found);
};

/// @brief Represents an out-of-bound error.
class range_error : public json::parser_error {
public:
    const std::size_t index;
    const std::size_t size;

    /// @brief Construct a new range error.
    /// @param index the index we tried to access.
    /// @param size the size of the container.
    range_error(std::size_t _line, std::size_t _index, std::size_t _size);

private:
    static std::string build_message(std::size_t _index, std::size_t _size);
};

/// @brief JSON parser configuration.
namespace config
{
/// @brief If true, the library will throw an error if the C++ variable you
/// are using to read or write to a json node have different types.
extern bool strict_type_check;
/// @brief If true, the library will throw an error if the field of an
/// object you are trying to access does not exist.
extern bool strict_existance_check;
}; // namespace config

/// @brief Represent a json node.
class jnode_t {
public:
    /// The internal map of properties for JOBJECT nodes.
    typedef ordered_map::ordered_map_t<std::string, jnode_t> property_map_t;
    /// How properties are stored inside the internal map.
    typedef ordered_map::ordered_map_t<std::string, jnode_t>::list_entry_t property_t;
    /// The internal array of objects for JARRAY nodes.
    typedef std::vector<jnode_t> array_data_t;
    /// Sorting function for JARRAY.
    typedef bool (*sort_function_array_t)(const jnode_t &, const jnode_t &);
    /// Sorting function for JOBJECT.
    typedef bool (*sort_function_object_t)(const property_t &, const property_t &);

    /// @brief Constructor.
    jnode_t();

    /// @brief Constructor.
    /// @param _type The type to set.
    explicit jnode_t(jtype_t _type);

    /// @brief Returns the value of the json node.
    /// @return the unprocessed string contained in the node.
    std::string get_value() const;

    /// @brief Returns the type of the json node.
    /// @return the jtype_t of this node.
    jtype_t get_type() const;

    /// @brief Checks wheter the node is a JSTRING.
    /// @return true if the internal value is a string, false otherwise.
    bool is_string() const;

    /// @brief Checks wheter the node is a JBOOLEAN.
    /// @return true if the internal value is a bool, false otherwise.
    bool is_bool() const;

    /// @brief Checks wheter the node is a JARRAY.
    /// @return true if the node is an array of elements, false otherwise.
    bool is_array() const;

    /// @brief Checks wheter the node is a JOBJECT.
    /// @return true if the node is an object, false otherwise.
    bool is_object() const;

    /// @brief Checks wheter the node is a JNUMBER.
    /// @return true if the internal value is a number, false otherwise.
    bool is_number() const;

    /// @brief Checks wheter the node is a JNULL.
    /// @return true if the node contains is invalid, false otherwise.
    bool is_null() const;

    /// @brief Returns the line number where the object resides in the original code.
    /// @return the line number if the object was created by parsing a file, -1 otherwise.
    std::size_t get_line_number() const;

    /// @brief Returns the size of the internal array or the number of properties of the object.
    /// @return the size of the internal array or the number of properties of the object.
    std::size_t size() const;

    /// @brief Checks if the current object has the given property.
    /// @param key The key of the property.
    /// @return true if the object has the property, false otherwise.
    bool has_property(const std::string &key) const;

    /// @brief Turns the value to INT.
    /// @param check_type If true, check the correspondence of types.
    /// @return The extracted value
    template <typename T>
    T as_number() const
    {
        T output = 0;
        if (type == JNUMBER) {
            std::stringstream ss;
            ss << value;
            ss >> output;
        } else if (json::config::strict_type_check) {
            throw json::type_error(line_number, JNUMBER, type);
        }
        return output;
    }

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
    /// @return a reference to this object.
    jnode_t &set_type(jtype_t _type);

    /// @brief Sets the internal value.
    /// @param _value The value to set.
    /// @return a reference to this object.
    jnode_t &set_value(const std::string &_value);

    /// @brief Sets the line number.
    /// @param _line_number The line number to set.
    /// @return a reference to this object.
    jnode_t &set_line_number(std::size_t _line_number);

    /// @brief Adds a new property with the given key.
    /// @param key The key of the property.
    /// @return A reference to the newly created property.
    jnode_t &add_property(const std::string &key);

    /// @brief Ads a given property with the given key.
    /// @param key  The key of the property.
    /// @param node The property to set.
    /// @return A reference to the newly created property.
    jnode_t &add_property(const std::string &key, const jnode_t &node);

    /// @brief Remove the property with the given key.
    /// @param key The key of the property.
    void remove_property(const std::string &key);

    /// @brief Adds the element to the array.
    /// @param node The node to add.
    /// @return a reference to the added object.
    jnode_t &add_element(const jnode_t &node = jnode_t());

    /// @brief Removes an element from the array.
    /// @param index position of the element.
    void remove_element(std::size_t index);

    /// @brief Reserves the given space for the array.
    /// @param size the number of elements.
    void reserve(std::size_t size);

    /// @brief Resize the array to the given value and fills it with empty objects.
    /// @param size the number of elements.
    void resize(std::size_t size);

    /// @brief Clears all the internal data structures.
    void clear();

    /// @brief Allows to sort the entry inside the node, which be a JARRAY.
    /// @param fun The function used to sort.
    template <typename SortFunction>
    void sort(const SortFunction &fun)
    {
        if (type == JARRAY) {
            std::sort(arr.begin(), arr.end(), fun);
        } else if (type == JOBJECT) {
            properties.sort(fun);
        } else {
            throw json::parser_error(line_number, "You are trying to sort neither a JARRAY nor a JOBJECT");
        }
    }

    /// @brief Provides access to an internal node.
    /// @param i The index of the node.
    /// @return A const reference to the node. If this node is not
    ///          an array, or an object, returns a node of type JNULL.
    const jnode_t &operator[](std::size_t i) const;

    /// @brief Provides access to an internal node (THIS=Object/Array).
    /// @param i The index of the node.
    /// @return A reference to the node. If this node is not an array,
    ///          or an object, returns a node of type JNULL.
    jnode_t &operator[](std::size_t i);

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
    /// @return the string representation of the node.
    std::string to_string(bool pretty = true, unsigned tabsize = 4) const;

    /// @brief Returns a constant iterator pointing to the **beginning** of the **property map**.
    /// @return the iterator.
    property_map_t::const_iterator pbegin() const;

    /// @brief Returns an iterator pointing to the **beginning** of the **property map**.
    /// @return the iterator.
    property_map_t::iterator pbegin();

    /// @brief Returns a constant iterator pointing to the **end** of the **property map**.
    /// @return the iterator.
    property_map_t::const_iterator pend() const;

    /// @brief Returns an iterator pointing to the **end** of the **property map**.
    /// @return the iterator.
    property_map_t::iterator pend();

    /// @brief Returns a constant iterator pointing to the **beginning** of the **array**.
    /// @return the iterator.
    array_data_t::const_iterator abegin() const;

    /// @brief Returns an iterator pointing to the **beginning** of the **array**.
    /// @return the iterator.
    array_data_t::iterator abegin();

    /// @brief Returns a constant iterator pointing to the **end** of the **array**.
    /// @return the iterator.
    array_data_t::const_iterator aend() const;

    /// @brief Returns an iterator pointing to the **end** of the **array**.
    /// @return the iterator.
    array_data_t::iterator aend();

private:
    /// @brief Turns the json object to a formatted string.
    /// @param depth    The current depth, used for indentation (if pretty == true).
    /// @param pretty   Enable/Disable pretty print of json.
    /// @param tabsize	The dimension of tabulation (if pretty == true).
    /// @return the generated string.
    std::string to_string_d(unsigned depth, bool pretty = true, unsigned tabsize = 4) const;

    /// The type of the node.
    jtype_t type;
    /// The value contained inside the node.
    std::string value;
    /// The original line number.
    std::size_t line_number;
    /// The properties of the node.
    property_map_t properties;
    /// The array content.
    array_data_t arr;
};

/// @brief This namespace contains all sort of support function.
namespace detail
{

/// @brief The type of tokens we use to control parsing.
enum token_type_t {
    UNKNOWN,       ///< An unknown token.
    STRING,        ///< We are parsing a string.
    NUMBER,        ///< We are parsing a number.
    CURLY_OPEN,    ///< We found an open curly braket.
    CURLY_CLOSE,   ///< We found a close curly braket.
    BRACKET_OPEN,  ///< We found an open braket.
    BRACKET_CLOSE, ///< We found a close braket.
    COMMA,         ///< We found a comma.
    COLON,         ///< We found a colon.
    BOOLEAN,       ///< We found a boolean.
    NUL            ///< We found a NULL value.
};

/// @brief A token use for parsing.
typedef struct token_t {
    /// The value.
    std::string value;
    /// The type.
    token_type_t type;
    /// The line number.
    std::size_t line_number;

    /// @brief Creates a new token.
    /// @param _value the value contained in the token.
    /// @param _type the type of token.
    /// @param _line_number the line where the token was extracted from.
    token_t(const std::string &_value = "", token_type_t _type = UNKNOWN, std::size_t _line_number = 0)
        : value(_value),
          type(_type),
          line_number(_line_number)
    {
        // Nothing to do.
    }
} token_t;

/// @brief Parse the SOURCE string and extracts all of its tokens.
/// @param source the input string.
/// @param tokens a vector where we store the parsed tokens.
/// @return a reference to the token vector.
std::vector<token_t> &tokenize(const std::string &source, std::vector<token_t> &tokens);

/// @brief Parse the list of tokens into a JSON tree.
/// @param tokens the list of tokens.
/// @param index the internal index we use to handle tokens.
/// @param output_index the index we are currently dealing with.
/// @param current the current node we are building.
/// @return the generated json sub-tree.
jnode_t &json_parse(std::vector<token_t> &tokens, std::size_t index, std::size_t &output_index, jnode_t &current);

} // namespace detail

/// @brief Contains parsing functions.
namespace parser
{

/// @brief Parse the json formatted string.
/// @param json_string The json formatted string.
/// @return the root of the generated json tree.
jnode_t parse(const std::string &json_string);

/// @brief Parse the json file.
/// @param filename Path to the json file.
/// @return the root of the generated json tree.
bool read_file(const std::string &filename, std::string &content);

/// @brief Parse the json file.
/// @param filename Path to the json file.
/// @return the root of the generated json tree.
jnode_t parse_file(const std::string &filename);

/// @brief Write the json node on file.
/// @param filename Path to the json file.
/// @param node     The json tree to write.
/// @param pretty   Enable/Disable pretty print of json.
/// @param tabsize	The dimension of tabulation (if pretto == true).
/// @return if the operation is a success.
bool write_file(const std::string &filename, const jnode_t &node, bool pretty = true, unsigned tabsize = 4);

} // namespace parser

/// @brief Genering output writer.
/// @param lhs the JSON node we are writing into.
/// @param rhs the value we are writing into the JSON node.
/// @return a reference to the JSON node.
template <typename T>
json::jnode_t &operator<<(json::jnode_t &lhs, T const &rhs);

/// @brief Output writer for pointers.
/// @param lhs the JSON node we are writing into.
/// @param rhs the value we are writing into the JSON node.
/// @return a reference to the JSON node.
template <typename T>
inline json::jnode_t &operator<<(json::jnode_t &lhs, T *const &rhs)
{
    return lhs << (*rhs);
}

/// @brief Output writer for const char pointers.
/// @param lhs the JSON node we are writing into.
/// @param rhs the value we are writing into the JSON node.
/// @return a reference to the JSON node.
inline json::jnode_t &operator<<(json::jnode_t &lhs, char const *rhs)
{
    lhs.set_type(json::JSTRING);
    lhs.set_value(rhs);
    return lhs;
}

/// @brief Output writer for char pointers.
/// @param lhs the JSON node we are writing into.
/// @param rhs the value we are writing into the JSON node.
/// @return a reference to the JSON node.
inline json::jnode_t &operator<<(json::jnode_t &lhs, char *rhs)
{
    lhs.set_type(json::JSTRING);
    lhs.set_value(rhs);
    return lhs;
}

/// @brief Output writer for vectors.
/// @param lhs the JSON node we are writing into.
/// @param rhs the value we are writing into the JSON node.
/// @return a reference to the JSON node.
template <typename T>
inline json::jnode_t &operator<<(json::jnode_t &lhs, std::vector<T> const &rhs)
{
    lhs.clear();
    lhs.set_type(json::JARRAY);
    lhs.resize(rhs.size());
    for (std::size_t i = 0; i < rhs.size(); ++i) {
        lhs[i] << rhs[i];
    }
    return lhs;
}

#ifdef __cpp_lib_span
/// @brief Output writer for span.
/// @param lhs the JSON node we are writing into.
/// @param rhs the value we are writing into the JSON node.
/// @return a reference to the JSON node.
template <typename T>
inline json::jnode_t &operator<<(json::jnode_t &lhs, std::span<T> rhs)
{
    lhs.clear();
    lhs.set_type(json::JARRAY);
    lhs.resize(rhs.size());
    for (std::size_t i = 0; i < rhs.size(); ++i) {
        lhs[i] << rhs[i];
    }
    return lhs;
}
#endif

/// @brief Output writer for lists.
/// @param lhs the JSON node we are writing into.
/// @param rhs the value we are writing into the JSON node.
/// @return a reference to the JSON node.
template <typename T>
inline json::jnode_t &operator<<(json::jnode_t &lhs, std::list<T> const &rhs)
{
    lhs.clear();
    lhs.set_type(json::JARRAY);
    lhs.resize(rhs.size());
    std::size_t i = 0;
    for (typename std::list<T>::const_iterator it = rhs.begin(); it != rhs.end(); ++it) {
        lhs[i++] << (*it);
    }
    return lhs;
}

/// @brief Output writer for sets.
/// @param lhs the JSON node we are writing into.
/// @param rhs the value we are writing into the JSON node.
/// @return a reference to the JSON node.
template <typename T>
inline json::jnode_t &operator<<(json::jnode_t &lhs, std::set<T> const &rhs)
{
    lhs.clear();
    lhs.set_type(json::JARRAY);
    lhs.resize(rhs.size());
    std::size_t i = 0;
    for (typename std::set<T>::const_iterator it = rhs.begin(); it != rhs.end(); ++it) {
        lhs[i++] << (*it);
    }
    return lhs;
}

/// @brief Output writer for maps.
/// @param lhs the JSON node we are writing into.
/// @param rhs the value we are writing into the JSON node.
/// @return a reference to the JSON node.
template <typename T1, typename T2>
inline json::jnode_t &operator<<(json::jnode_t &lhs, std::map<T1, T2> const &rhs)
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

/// @brief Genering input reader.
/// @param lhs the JSON node we are reading from.
/// @param rhs the value we are storing the JSON node content.
/// @return a const reference to the JSON node.
template <typename T>
const json::jnode_t &operator>>(const json::jnode_t &lhs, T &rhs);

/// @brief Input reader for pointers.
/// @param lhs the JSON node we are reading from.
/// @param rhs the value we are storing the JSON node content.
/// @return a const reference to the JSON node.
template <typename T>
inline const json::jnode_t &operator>>(const json::jnode_t &lhs, T *&rhs)
{
    return lhs >> (*rhs);
}

/// @brief Input reader for vectors.
/// @param lhs the JSON node we are reading from.
/// @param rhs the value we are storing the JSON node content.
/// @return a const reference to the JSON node.
template <typename T>
inline const json::jnode_t &operator>>(const json::jnode_t &lhs, std::vector<T> &rhs)
{
    if (lhs.get_type() == json::JARRAY) {
        // Clear the vector.
        rhs.clear();
        // Resize the vector.
        rhs.resize(lhs.size());
        // Load its elements.
        for (std::size_t i = 0; i < lhs.size(); ++i) {
            lhs[i] >> rhs[i];
        }
    }
    return lhs;
}

#ifdef __cpp_lib_span

/// @brief Input reader for spans.
/// @param lhs the JSON node we are reading from.
/// @param rhs the value we are storing the JSON node content.
/// @return a const reference to the JSON node.
template <typename T>
inline const json::jnode_t &operator>>(const json::jnode_t &lhs, std::span<T> rhs)
{
    if (lhs.get_type() == json::JARRAY) {
        assert(lhs.size() <= rhs.size());
        // NOTE: This should not be necessary (see assert above) but for safety reasons, ensure there is no out of bounds acces
        const std::size_t elem_count = lhs.size() < rhs.size() ? lhs.size() : rhs.size();
        for (std::size_t i = 0; i < elem_count; ++i) {
            lhs[i] >> rhs[i];
        }
    }
    return lhs;
}
#endif

/// @brief Input reader for lists.
/// @param lhs the JSON node we are reading from.
/// @param rhs the value we are storing the JSON node content.
/// @return a const reference to the JSON node.
template <typename T>
inline const json::jnode_t &operator>>(const json::jnode_t &lhs, std::list<T> &rhs)
{
    if (lhs.get_type() == json::JARRAY) {
        rhs.clear();
        rhs.resize(lhs.size());
        std::size_t i = 0;
        for (typename std::list<T>::iterator it = rhs.begin(); it != rhs.end(); ++it) {
            lhs[i++] >> (*it);
        }
    }
    return lhs;
}

/// @brief Input reader for sets.
/// @param lhs the JSON node we are reading from.
/// @param rhs the value we are storing the JSON node content.
/// @return a const reference to the JSON node.
template <typename T>
inline const json::jnode_t &operator>>(const json::jnode_t &lhs, std::set<T> &rhs)
{
    if (lhs.get_type() == json::JARRAY) {
        rhs.clear();
        for (std::size_t i = 0; i < lhs.size(); ++i) {
            T t;
            lhs[i] >> t;
            rhs.insert(t);
        }
    }
    return lhs;
}

/// @brief Input reader for maps.
/// @param lhs the JSON node we are reading from.
/// @param rhs the value we are storing the JSON node content.
/// @return a const reference to the JSON node.
template <typename T1, typename T2>
inline const json::jnode_t &operator>>(const json::jnode_t &lhs, std::map<T1, T2> &rhs)
{
    // Check the type.
    if (lhs.get_type() == json::JOBJECT) {
        rhs.clear();
        json::jnode_t::property_map_t::const_iterator it;
        for (it = lhs.pbegin(); it != lhs.pend(); ++it) {
            std::stringstream ss;
            ss << it->first;
            T1 key;
            ss >> key;
            it->second >> rhs[key];
        }
    }
    return lhs;
}

} // namespace json

/// @brief Define the pair of operators required to handle C++ enums.
#define JSON_DEFINE_OP_ENUM(ENUM_TYPE)                                        \
    namespace json                                                            \
    {                                                                         \
    template <>                                                               \
    json::jnode_t &operator<<(json::jnode_t &lhs, const ENUM_TYPE &rhs)       \
    {                                                                         \
        return (lhs << static_cast<int>(rhs));                                \
    }                                                                         \
    template <>                                                               \
    const json::jnode_t &operator>>(const json::jnode_t &lhs, ENUM_TYPE &rhs) \
    {                                                                         \
        rhs = static_cast<ENUM_TYPE>(lhs.as_number<int>());                   \
        return lhs;                                                           \
    }                                                                         \
    }

/// @brief Sends the JSON node to the output stream.
/// @param lhs the stream we are writing the content of the JSON node.
/// @param rhs the JSON node.
/// @return a reference to the output stream.
std::ostream &operator<<(std::ostream &lhs, const json::jnode_t &rhs);

/// @brief Sends the JSON node to the output file stream.
/// @param lhs the stream we are writing the content of the JSON node.
/// @param rhs the JSON node.
/// @return a reference to the output file stream.
std::ofstream &operator<<(std::ofstream &lhs, const json::jnode_t &rhs);
