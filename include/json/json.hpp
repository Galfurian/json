/// @file json.hpp
/// @author Enrico Fraccaroli (enry.frak@gmail.com)
/// @brief Defines the jnode_t class.
///
/// @copyright (c) 2024 This file is distributed under the MIT License.
/// See LICENSE.md for details.
///

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
#include <memory>
#include <complex>
#include <deque>
#include <unordered_map>
#include <bitset>

#ifdef __cpp_lib_span
#include <span>
#endif

#include "ordered_map/ordered_map.hpp"

/// @brief This namespace contains the main json_t class and stream functions.
namespace json
{

/// @brief JSON types.
enum jtype_t {
    JTYPE_STRING,  ///< A string.
    JTYPE_OBJECT,  ///< An object.
    JTYPE_ARRAY,   ///< An array.
    JTYPE_BOOLEAN, ///< A boolean value.
    JTYPE_NUMBER,  ///< A number.
    JTYPE_NULL,    ///< A null value.
    JTYPE_ERROR    ///< An error.
};

/// @brief Transforms the given JSON type to string.
/// @param type the JSON type to transform to string.
/// @return the string representing the JSON type.
std::string jtype_to_string(jtype_t type);

/// @brief Represents a type error.
class parser_error : public std::runtime_error {
public:
    /// @brief The line where the error is located.
    const std::size_t line;

    /// @brief Construct a new type error.
    /// @param _line the line where the error was found.
    /// @param _message the error message.
    parser_error(std::size_t _line, std::string _message);
};

/// @brief Represents a type error.
class type_error : public json::parser_error {
public:
    /// @brief The expected type.
    const json::jtype_t expected;
    /// @brief The type we found.
    const json::jtype_t found;

    /// @brief Construct a new type error.
    /// @param _line the line where the error was found.
    /// @param _expected the expected type.
    /// @param _found the type we found.
    type_error(std::size_t _line, json::jtype_t _expected, json::jtype_t _found);

private:
    /// @brief Builds the error message.
    /// @param _expected the expected type.
    /// @param _found the type we found.
    /// @return the message.
    static std::string build_message(json::jtype_t _expected, json::jtype_t _found);
};

/// @brief Represents an out-of-bound error.
class range_error : public json::parser_error {
public:
    /// @brief The index we tried to access.
    const std::size_t index;
    /// @brief The size of the container.
    const std::size_t size;

    /// @brief Construct a new range error.
    /// @param _line the line where the error was found.
    /// @param _index the index we tried to access.
    /// @param _size the size of the container.
    range_error(std::size_t _line, std::size_t _index, std::size_t _size);

private:
    /// @brief Builds the error message.
    /// @param _index the index we tried to access.
    /// @param _size the size of the container.
    /// @return the message.
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
/// @brief If true, the library will replace escape character when printing to
/// output.
extern bool replace_escape_characters;
/// @brief The character used to delimit strings.
extern char string_delimiter_character;
} // namespace config

/// @brief Represent a json node.
class jnode_t {
public:
    /// The internal map of properties for JTYPE_OBJECT nodes.
    typedef ordered_map::ordered_map_t<std::string, jnode_t> property_map_t;
    /// How properties are stored inside the internal map.
    typedef ordered_map::ordered_map_t<std::string, jnode_t>::list_entry_t property_t;
    /// The internal array of objects for JTYPE_ARRAY nodes.
    typedef std::vector<jnode_t> array_data_t;
    /// Sorting function for JTYPE_ARRAY.
    typedef bool (*sort_function_array_t)(const jnode_t &, const jnode_t &);
    /// Sorting function for JTYPE_OBJECT.
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

    /// @brief Checks wheter the node is a JTYPE_STRING.
    /// @return true if the internal value is a string, false otherwise.
    bool is_string() const;

    /// @brief Checks wheter the node is a JTYPE_BOOLEAN.
    /// @return true if the internal value is a bool, false otherwise.
    bool is_bool() const;

    /// @brief Checks wheter the node is a JTYPE_ARRAY.
    /// @return true if the node is an array of elements, false otherwise.
    bool is_array() const;

    /// @brief Checks wheter the node is a JTYPE_OBJECT.
    /// @return true if the node is an object, false otherwise.
    bool is_object() const;

    /// @brief Checks wheter the node is a JTYPE_NUMBER.
    /// @return true if the internal value is a number, false otherwise.
    bool is_number() const;

    /// @brief Checks wheter the node is a JTYPE_NULL.
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
        if (type == JTYPE_NUMBER) {
            std::stringstream ss;
            ss << value;
            ss >> output;
        } else if (json::config::strict_type_check) {
            throw json::type_error(line_number, JTYPE_NUMBER, type);
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

    /// @brief Allows to sort the entry inside the node, which be a JTYPE_ARRAY.
    /// @param fun The function used to sort.
    template <typename SortFunction>
    void sort(const SortFunction &fun)
    {
        if (type == JTYPE_ARRAY) {
            std::sort(arr.begin(), arr.end(), fun);
        } else if (type == JTYPE_OBJECT) {
            properties.sort(fun);
        } else {
            throw json::parser_error(line_number, "You are trying to sort neither a JTYPE_ARRAY nor a JTYPE_OBJECT");
        }
    }

    /// @brief Provides access to an internal node.
    /// @param i The index of the node.
    /// @return A const reference to the node. If this node is not
    ///          an array, or an object, returns a node of type JTYPE_NULL.
    const jnode_t &operator[](std::size_t i) const;

    /// @brief Provides access to an internal node (THIS=Object/Array).
    /// @param i The index of the node.
    /// @return A reference to the node. If this node is not an array,
    ///          or an object, returns a node of type JTYPE_NULL.
    jnode_t &operator[](std::size_t i);

    /// @brief Provides access to an internal node.
    /// @param key The key of the internal node.
    /// @return A const reference to the node. If this node is not
    ///          an array, or an object, returns a node of type JTYPE_NULL.
    const jnode_t &operator[](const std::string &key) const;

    /// @brief Provides access to an internal node.
    /// @param key The key of the internal node.
    /// @return A reference to the node. If this node is not
    ///          an array, or an object, returns a node of type JTYPE_NULL.
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
    JTOKEN_UNKNOWN,       ///< An unknown token.
    JTOKEN_STRING,        ///< We are parsing a string.
    JTOKEN_NUMBER,        ///< We are parsing a number.
    JTOKEN_CURLY_OPEN,    ///< We found an open curly braket.
    JTOKEN_CURLY_CLOSE,   ///< We found a close curly braket.
    JTOKEN_BRACKET_OPEN,  ///< We found an open braket.
    JTOKEN_BRACKET_CLOSE, ///< We found a close braket.
    JTOKEN_COMMA,         ///< We found a comma.
    JTOKEN_COLON,         ///< We found a colon.
    JTOKEN_BOOLEAN,       ///< We found a boolean.
    JTOKEN_NULL           ///< We found a NULL value.
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
    explicit token_t(const std::string &_value = "", token_type_t _type = JTOKEN_UNKNOWN, std::size_t _line_number = 0)
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

/// @brief Struct for transforming a std::tuple into a json::jnode_t.
/// @details
/// This struct provides a static method `transform` that recursively transforms
/// each element of a std::tuple into a json::jnode_t object. The transformation
/// starts from the element at index N and continues until the element at index Last.
/// @tparam Type The type of the std::tuple.
/// @tparam N The index of the current element being transformed.
/// @tparam Last The index of the last element in the tuple.
template <typename Type, std::size_t N, std::size_t Last>
struct tuple_to_json {
    /// @brief Transform an element of a the std::tuple into a json::jnode_t.
    /// @details
    /// This method recursively transforms each element of the std::tuple into
    /// a json::jnode_t object. It starts from the element at index N and
    /// continues until the element at index Last.
    /// @param lhs The json::jnode_t object where the transformation result will be stored.
    /// @param rhs The std::tuple to be transformed.
    static void transform(json::jnode_t &lhs, const Type &rhs)
    {
        lhs[N] << std::get<N>(rhs);
        tuple_to_json<Type, N + 1, Last>::transform(lhs, rhs);
    }
};

/// @brief Struct for transforming a std::tuple into a json::jnode_t.
/// @details
/// This specialization provides a static method `transform` for the base case
/// of the transformation, where N equals Last. It transforms the last element
/// of the std::tuple into a json::jnode_t object.
/// @tparam Type The type of the std::tuple.
/// @tparam N The index of the current element being transformed.
template <typename Type, std::size_t N>
struct tuple_to_json<Type, N, N> {
    /// @brief Transform the last element of the std::tuple into a json::jnode_t.
    /// @details
    /// This method transforms the last element of the std::tuple into a json::jnode_t
    /// object. It sets the value of the json::jnode_t at index N to the value of the
    /// corresponding element in the std::tuple.
    /// @param lhs The json::jnode_t object where the transformation result will be stored.
    /// @param rhs The std::tuple to be transformed.
    static void transform(json::jnode_t &lhs, const Type &rhs)
    {
        lhs[N] << std::get<N>(rhs);
    }
};

/// @brief Struct for transforming a json::jnode_t into a std::tuple.
/// @details
/// This struct provides a static method `transform` that recursively transforms
/// each element of a json::jnode_t object into a std::tuple. The transformation
/// starts from the element at index N and continues until the element at index Last.
/// @tparam Type The type of the std::tuple.
/// @tparam N The index of the current element being transformed.
/// @tparam Last The index of the last element in the tuple.
template <typename Type, std::size_t N, std::size_t Last>
struct json_to_tuple {
    /// @brief Transform the json::jnode_t into a std::tuple.
    /// @details
    /// This method recursively transforms each element of the json::jnode_t object into
    /// a corresponding element of the std::tuple. It starts from the element at index N and
    /// continues until the element at index Last.
    /// @param lhs The json::jnode_t object to be transformed.
    /// @param rhs The std::tuple where the transformation result will be stored.
    static void transform(const json::jnode_t &lhs, Type &rhs)
    {
        lhs[N] >> std::get<N>(rhs);
        json_to_tuple<Type, N + 1, Last>::transform(lhs, rhs);
    }
};

/// @brief Specialization of json_to_tuple struct for the base case.
/// @details
/// This specialization provides a static method `transform` for the base case
/// of the transformation, where N equals Last. It transforms the last element
/// of the json::jnode_t object into the corresponding element of the std::tuple.
/// @tparam Type The type of the std::tuple.
/// @tparam N The index of the current element being transformed.
template <typename Type, std::size_t N>
struct json_to_tuple<Type, N, N> {
    /// @brief Transform the last element of the json::jnode_t into the std::tuple.
    /// @details
    /// This method transforms the last element of the json::jnode_t object into
    /// the corresponding element of the std::tuple. It sets the value of the
    /// corresponding element in the std::tuple to the value of the json::jnode_t at index N.
    /// @param lhs The json::jnode_t object to be transformed.
    /// @param rhs The std::tuple where the transformation result will be stored.
    static void transform(const json::jnode_t &lhs, Type &rhs)
    {
        lhs[N] >> std::get<N>(rhs);
    }
};

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
/// @param content where we need to place the content of the file.
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
template <typename T, typename std::enable_if<!std::is_enum<T>::value, int>::type = 0>
json::jnode_t &operator<<(json::jnode_t &lhs, T const &rhs);

/// @brief Genering output writer.
/// @param lhs the JSON node we are writing into.
/// @param rhs the value we are writing into the JSON node.
/// @return a reference to the JSON node.
template <typename T, typename std::enable_if<std::is_enum<T>::value, int>::type = 0>
json::jnode_t &operator<<(json::jnode_t &lhs, T rhs)
{
    return lhs << static_cast<int>(rhs);
}

/// @brief Output writer for pointers.
/// @param lhs the JSON node we are writing into.
/// @param rhs the value we are writing into the JSON node.
/// @return a reference to the JSON node.
template <typename T>
inline json::jnode_t &operator<<(json::jnode_t &lhs, T *const &rhs)
{
    return lhs << (*rhs);
}

/// @brief Output writer for pointers.
/// @param lhs the JSON node we are writing into.
/// @param rhs the value we are writing into the JSON node.
/// @return a reference to the JSON node.
template <typename T>
inline json::jnode_t &operator<<(json::jnode_t &lhs, std::shared_ptr<T> const &rhs)
{
    return lhs << (*rhs);
}

/// @brief Output writer for const char pointers.
/// @param lhs the JSON node we are writing into.
/// @param rhs the value we are writing into the JSON node.
/// @return a reference to the JSON node.
inline json::jnode_t &operator<<(json::jnode_t &lhs, char const *rhs)
{
    lhs.set_type(json::JTYPE_STRING);
    lhs.set_value(rhs);
    return lhs;
}

/// @brief Output writer for char pointers.
/// @param lhs the JSON node we are writing into.
/// @param rhs the value we are writing into the JSON node.
/// @return a reference to the JSON node.
inline json::jnode_t &operator<<(json::jnode_t &lhs, char *rhs)
{
    lhs.set_type(json::JTYPE_STRING);
    lhs.set_value(rhs);
    return lhs;
}

/// @brief Output writer for complex types.
/// @param lhs the JSON node we are writing into.
/// @param rhs the value we are writing into the JSON node.
/// @return a reference to the JSON node.
template <typename T>
inline json::jnode_t &operator<<(json::jnode_t &lhs, const std::complex<T> &rhs)
{
    lhs.set_type(json::JTYPE_OBJECT);
    lhs["real"] << rhs.real();
    lhs["imag"] << rhs.imag();
    return lhs;
}

/// @brief Output writer for pairs.
/// @param lhs the JSON node we are writing into.
/// @param rhs the value we are writing into the JSON node.
/// @return a reference to the JSON node.
template <typename T1, typename T2>
inline json::jnode_t &operator<<(json::jnode_t &lhs, const std::pair<T1, T2> &rhs)
{
    lhs.set_type(json::JTYPE_OBJECT);
    lhs["first"] << rhs.first;
    lhs["second"] << rhs.second;
    return lhs;
}

/// @brief Overloaded output stream operator for streaming a std::tuple into a json::jnode_t.
/// @details
/// This operator streams the elements of a std::tuple into a json::jnode_t object.
/// It sets the type of the json::jnode_t object to JTYPE_ARRAY and resizes it to the
/// size of the tuple. Then, it calls the `transform` method of the `tuple_to_json` struct
/// to transform each element of the tuple into a corresponding element of the json::jnode_t.
/// @tparam Types The types of the elements in the std::tuple.
/// @param lhs The json::jnode_t object where the tuple elements will be streamed.
/// @param rhs The std::tuple to be streamed into the json::jnode_t.
/// @return A reference to the json::jnode_t object after streaming the tuple.
template <typename... Types>
inline json::jnode_t &operator<<(json::jnode_t &lhs, const std::tuple<Types...> &rhs)
{
    constexpr std::size_t tuple_size = sizeof...(Types);
    if (tuple_size > 0) {
        lhs.set_type(json::JTYPE_ARRAY);
        lhs.resize(tuple_size);
        detail::tuple_to_json<std::tuple<Types...>, 0, tuple_size - 1>::transform(lhs, rhs);
    }
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
    lhs.set_type(json::JTYPE_ARRAY);
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
    lhs.set_type(json::JTYPE_ARRAY);
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
    lhs.set_type(json::JTYPE_ARRAY);
    lhs.resize(rhs.size());
    std::size_t i = 0;
    for (typename std::list<T>::const_iterator it = rhs.begin(); it != rhs.end(); ++it) {
        lhs[i++] << (*it);
    }
    return lhs;
}

/// @brief Output writer for arrays.
/// @param lhs the JSON node we are writing into.
/// @param rhs the value we are writing into the JSON node.
/// @return a reference to the JSON node.
template <typename T, std::size_t N>
inline json::jnode_t &operator<<(json::jnode_t &lhs, std::array<T, N> const &rhs)
{
    lhs.clear();
    lhs.set_type(json::JTYPE_ARRAY);
    lhs.resize(rhs.size());
    for (std::size_t i = 0; i < N; ++i) {
        lhs[i] << rhs[i];
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
    lhs.set_type(json::JTYPE_ARRAY);
    lhs.resize(rhs.size());
    std::size_t i = 0;
    for (typename std::set<T>::const_iterator it = rhs.begin(); it != rhs.end(); ++it) {
        lhs[i++] << (*it);
    }
    return lhs;
}

/// @brief Output writer for maps with NOT an enum as key.
/// @param lhs the JSON node we are writing into.
/// @param rhs the value we are writing into the JSON node.
/// @return a reference to the JSON node.
template <typename T1, typename T2, typename std::enable_if<!std::is_enum<T1>::value, int>::type = 0>
inline json::jnode_t &operator<<(json::jnode_t &lhs, const std::map<T1, T2> &rhs)
{
    // Clear the current contents of the JSON node.
    lhs.clear();
    // Set the type to object.
    lhs.set_type(json::JTYPE_OBJECT);
    for (const auto &pair : rhs) {
        std::stringstream ss;
        // Convert the key to string.
        ss << pair.first;
        // Serialize the value.
        lhs.add_property(ss.str()) << pair.second;
    }
    return lhs; // Return the updated JSON node
}

/// @brief Output writer for maps with enum as key.
/// @param lhs the JSON node we are writing into.
/// @param rhs the value we are writing into the JSON node.
/// @return a reference to the JSON node.
template <typename T1, typename T2, typename std::enable_if<std::is_enum<T1>::value, int>::type = 0>
inline json::jnode_t &operator<<(json::jnode_t &lhs, const std::map<T1, T2> &rhs)
{
    // Clear the current contents of the JSON node.
    lhs.clear();
    // Set the type to object.
    lhs.set_type(json::JTYPE_OBJECT);
    for (const auto &pair : rhs) {
        std::stringstream ss;
        // Convert the key to string.
        ss << static_cast<int>(pair.first);
        // Serialize the value.
        lhs.add_property(ss.str()) << pair.second;
    }
    // Return the updated JSON node.
    return lhs;
}

/// @brief Output writer for unordered maps with NOT an enum as key.
/// @param lhs the JSON node we are writing into.
/// @param rhs the value we are writing into the JSON node.
/// @return a reference to the JSON node.
template <typename T1, typename T2, typename std::enable_if<!std::is_enum<T1>::value, int>::type = 0>
inline json::jnode_t &operator<<(json::jnode_t &lhs, const std::unordered_map<T1, T2> &rhs)
{
    // Clear the current contents of the JSON node.
    lhs.clear();
    // Set the type to object.
    lhs.set_type(json::JTYPE_OBJECT);
    for (const auto &pair : rhs) {
        std::stringstream ss;
        // Convert the key to string.
        ss << pair.first;
        // Serialize the value.
        lhs.add_property(ss.str()) << pair.second;
    }
    // Return the updated JSON node.
    return lhs;
}

/// @brief Output writer for unordered maps with enum as key.
/// @param lhs the JSON node we are writing into.
/// @param rhs the value we are writing into the JSON node.
/// @return a reference to the JSON node.
template <typename T1, typename T2, typename std::enable_if<std::is_enum<T1>::value, int>::type = 0>
inline json::jnode_t &operator<<(json::jnode_t &lhs, const std::unordered_map<T1, T2> &rhs)
{
    // Clear the current contents of the JSON node.
    lhs.clear();
    // Set the type to object.
    lhs.set_type(json::JTYPE_OBJECT);
    for (const auto &pair : rhs) {
        std::stringstream ss;
        // Convert the key to string.
        ss << static_cast<int>(pair.first);
        // Serialize the value.
        lhs.add_property(ss.str()) << pair.second;
    }
    // Return the updated JSON node.
    return lhs;
}

/// @brief Overloaded output stream operator for serializing std::deque into a json::jnode_t.
/// @param lhs The json::jnode_t object where the deque elements will be streamed.
/// @param rhs The std::deque to be streamed into the json::jnode_t.
/// @return A reference to the json::jnode_t object after streaming the deque.
template <typename T>
inline json::jnode_t &operator<<(json::jnode_t &lhs, const std::deque<T> &rhs)
{
    // Clear the node.
    lhs.clear();
    // Set the type to array.
    lhs.set_type(json::JTYPE_ARRAY);
    // Resize the JSON node to match the size of the deque.
    lhs.resize(rhs.size());
    // Serialize each element of the deque into the JSON node.
    for (std::size_t i = 0; i < rhs.size(); ++i) {
        lhs[i] << rhs[i];
    }
    // Return the updated JSON node.
    return lhs;
}

/// @brief Output writer for std::bitset.
/// @param lhs the JSON node we are writing into.
/// @param rhs the std::bitset to be written into the JSON node.
/// @return a reference to the JSON node.
template <std::size_t N>
inline json::jnode_t &operator<<(json::jnode_t &lhs, const std::bitset<N> &rhs)
{
    // Set the type to string.
    lhs.set_type(json::JTYPE_STRING);
    // Convert the bitset to string and set it as the value.
    lhs.set_value(rhs.to_string());
    // Return the updated JSON node.
    return lhs;
}

/// @brief Genering input reader.
/// @param lhs the JSON node we are reading from.
/// @param rhs the value we are storing the JSON node content.
/// @return a const reference to the JSON node.
template <typename T, typename std::enable_if<!std::is_enum<T>::value, int>::type = 0>
const json::jnode_t &operator>>(const json::jnode_t &lhs, T &rhs);

/// @brief Genering input reader.
/// @param lhs the JSON node we are reading from.
/// @param rhs the value we are storing the JSON node content.
/// @return a const reference to the JSON node.
template <typename T, typename std::enable_if<std::is_enum<T>::value, int>::type = 0>
const json::jnode_t &operator>>(const json::jnode_t &lhs, T &rhs)
{
    // Convert the JSON node's integer representation to the enum type
    rhs = static_cast<T>(lhs.as_number<int>());
    return lhs;
}

/// @brief Input reader for pointers.
/// @param lhs the JSON node we are reading from.
/// @param rhs the value we are storing the JSON node content.
/// @return a const reference to the JSON node.
template <typename T>
inline const json::jnode_t &operator>>(const json::jnode_t &lhs, T *&rhs)
{
    return lhs >> (*rhs);
}

/// @brief Input reader for complex types.
/// @param lhs the JSON node we are reading from.
/// @param rhs the value we are storing the JSON node content.
/// @return a const reference to the JSON node.
template <typename T>
inline const json::jnode_t &operator>>(const json::jnode_t &lhs, std::complex<T> &rhs)
{
    if (lhs.get_type() == json::JTYPE_OBJECT) {
        T real, imag;
        lhs["real"] >> real;
        lhs["imag"] >> imag;
        rhs.real(real);
        rhs.imag(imag);
    }
    return lhs;
}

/// @brief Input reader for pairs.
/// @param lhs the JSON node we are reading from.
/// @param rhs the value we are storing the JSON node content.
/// @return a const reference to the JSON node.
template <typename T1, typename T2>
inline const json::jnode_t &operator>>(const json::jnode_t &lhs, std::pair<T1, T2> &rhs)
{
    if (lhs.get_type() == json::JTYPE_OBJECT) {
        lhs["first"] >> rhs.first;
        lhs["second"] >> rhs.second;
    }
    return lhs;
}

/// @brief Overloaded input stream operator for reading a std::tuple from a json::jnode_t.
/// @details
/// This operator reads the elements of a json::jnode_t object into a std::tuple.
/// It checks if the json::jnode_t object is of type JTYPE_ARRAY and its size matches
/// the size of the tuple. If so, it calls the `transform` method of the `json_to_tuple`
/// struct to transform each element of the json::jnode_t into a corresponding element
/// of the std::tuple.
/// @tparam Types The types of the elements in the std::tuple.
/// @param lhs The json::jnode_t object to be read into the std::tuple.
/// @param rhs The std::tuple where the json::jnode_t elements will be stored.
/// @return A reference to the json::jnode_t object after reading into the tuple.
template <typename... Types>
inline const json::jnode_t &operator>>(const json::jnode_t &lhs, std::tuple<Types...> &rhs)
{
    constexpr std::size_t tuple_size = sizeof...(Types);
    if ((lhs.get_type() == json::JTYPE_ARRAY) && (lhs.size() == tuple_size) && (tuple_size > 0)) {
        detail::json_to_tuple<std::tuple<Types...>, 0, tuple_size - 1>::transform(lhs, rhs);
    }
    return lhs;
}

/// @brief Input reader for vectors.
/// @param lhs the JSON node we are reading from.
/// @param rhs the value we are storing the JSON node content.
/// @return a const reference to the JSON node.
template <typename T>
inline const json::jnode_t &operator>>(const json::jnode_t &lhs, std::vector<T> &rhs)
{
    if (lhs.get_type() == json::JTYPE_ARRAY) {
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
    if (lhs.get_type() == json::JTYPE_ARRAY) {
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
    if (lhs.get_type() == json::JTYPE_ARRAY) {
        rhs.clear();
        rhs.resize(lhs.size());
        std::size_t i = 0;
        for (typename std::list<T>::iterator it = rhs.begin(); it != rhs.end(); ++it) {
            lhs[i++] >> (*it);
        }
    }
    return lhs;
}

/// @brief Input reader for arrays.
/// @param lhs the JSON node we are reading from.
/// @param rhs the value we are storing the JSON node content.
/// @return a const reference to the JSON node.
template <typename T, std::size_t N>
inline const json::jnode_t &operator>>(const json::jnode_t &lhs, std::array<T, N> &rhs)
{
    if ((lhs.get_type() == json::JTYPE_ARRAY) && (lhs.size() == N)) {
        for (std::size_t i = 0; i < N; ++i) {
            lhs[i] >> rhs[i];
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
    if (lhs.get_type() == json::JTYPE_ARRAY) {
        rhs.clear();
        for (std::size_t i = 0; i < lhs.size(); ++i) {
            T t;
            lhs[i] >> t;
            rhs.insert(t);
        }
    }
    return lhs;
}

/// @brief Input reader for maps, with NOT an enum as key.
/// @param lhs the JSON node we are reading from.
/// @param rhs the value we are storing the JSON node content.
/// @return a const reference to the JSON node.
template <typename T1, typename T2, typename std::enable_if<!std::is_enum<T1>::value, int>::type = 0>
inline const json::jnode_t &operator>>(const json::jnode_t &lhs, std::map<T1, T2> &rhs)
{
    // Check the type.
    if (lhs.get_type() == json::JTYPE_OBJECT) {
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

/// @brief Input reader for maps, with enum as key.
/// @param lhs the JSON node we are reading from.
/// @param rhs the value we are storing the JSON node content.
/// @return a const reference to the JSON node.
template <typename T1, typename T2, typename std::enable_if<std::is_enum<T1>::value, int>::type = 0>
inline const json::jnode_t &operator>>(const json::jnode_t &lhs, std::map<T1, T2> &rhs)
{
    // Check the type.
    if (lhs.get_type() == json::JTYPE_OBJECT) {
        rhs.clear();
        json::jnode_t::property_map_t::const_iterator it;
        for (it = lhs.pbegin(); it != lhs.pend(); ++it) {
            std::stringstream ss;
            ss << it->first;
            int key;
            ss >> key;
            it->second >> rhs[T1(key)];
        }
    }
    return lhs;
}

/// @brief Input reader for unordered maps with NOT an enum as key.
/// @param lhs the JSON node we are reading from.
/// @param rhs the value we are storing the JSON node content.
/// @return a const reference to the JSON node.
template <typename T1, typename T2, typename std::enable_if<!std::is_enum<T1>::value, int>::type = 0>
inline const json::jnode_t &operator>>(const json::jnode_t &lhs, std::unordered_map<T1, T2> &rhs)
{
    // Check the type.
    if (lhs.get_type() == json::JTYPE_OBJECT) {
        rhs.clear();
        for (auto it = lhs.pbegin(); it != lhs.pend(); ++it) {
            std::stringstream ss;
            // Convert key to string.
            ss << it->first;
            T1 key;
            // Convert string back to key type.
            ss >> key;
            // Deserialize the corresponding value.
            it->second >> rhs[key];
        }
    }
    // Return the original JSON node.
    return lhs;
}

/// @brief Input reader for unordered maps with enum as key.
/// @param lhs the JSON node we are reading from.
/// @param rhs the value we are storing the JSON node content.
/// @return a const reference to the JSON node.
template <typename T1, typename T2, typename std::enable_if<std::is_enum<T1>::value, int>::type = 0>
inline const json::jnode_t &operator>>(const json::jnode_t &lhs, std::unordered_map<T1, T2> &rhs)
{
    // Check the type.
    if (lhs.get_type() == json::JTYPE_OBJECT) {
        rhs.clear();
        for (auto it = lhs.pbegin(); it != lhs.pend(); ++it) {
            std::stringstream ss;
            // Convert key to string.
            ss << it->first;
            int key;
            // Convert string back to key type.
            ss >> key;
            // Deserialize the corresponding value.
            it->second >> rhs[T1(key)];
        }
    }
    // Return the original JSON node.
    return lhs;
}

/// @brief Overloaded input stream operator for deserializing std::deque from a json::jnode_t.
/// @param lhs The json::jnode_t object to read from.
/// @param rhs The std::deque where the json::jnode_t elements will be stored.
/// @return A const reference to the json::jnode_t object after reading into the deque.
template <typename T>
inline const json::jnode_t &operator>>(const json::jnode_t &lhs, std::deque<T> &rhs)
{
    if (lhs.get_type() == json::JTYPE_ARRAY) {
        // Clear the existing elements in the deque.
        rhs.clear();
        // Resize the deque to match the size of the JSON array.
        rhs.resize(lhs.size());
        // Deserialize each element from the JSON node into the deque.
        for (std::size_t i = 0; i < lhs.size(); ++i) {
            lhs[i] >> rhs[i];
        }
    }
    // Return the original JSON node.
    return lhs;
}

/// @brief Input reader for std::bitset.
/// @param lhs the JSON node we are reading from.
/// @param rhs the std::bitset where the value will be stored.
/// @return a const reference to the JSON node.
template <std::size_t N>
inline const json::jnode_t &operator>>(const json::jnode_t &lhs, std::bitset<N> &rhs)
{
    // Check if the type is string.
    if (lhs.get_type() == json::JTYPE_STRING) {
        // Get the string value.
        std::string value = lhs.get_value();
        if (value.size() != N) {
            throw std::invalid_argument("Bitset size mismatch. Expected: " + std::to_string(N) + ", but got: " + std::to_string(value.size()));
        }
        // Convert string to bitset
        rhs = std::bitset<N>(value);
    }
    // Return the original JSON node.
    return lhs;
}

} // namespace json

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
