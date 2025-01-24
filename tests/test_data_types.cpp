/// @file test_data_types.cpp
/// @author Enrico Fraccaroli (enry.frak@gmail.com)
/// @brief Tests compatibility of the json library with different data types.
///
/// @copyright (c) 2024 This file is distributed under the MIT License.
/// See LICENSE.md for details.
///

#include <json/json.hpp>

#include <iostream>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <array>
#include <utility> // For std::pair
#include <tuple>   // For std::tuple
#include <string>

// Enum for testing
enum Color {
    RED,
    GREEN,
    BLUE
};

// Test functions for basic data types
int test_int()
{
    json::jnode_t json_node;
    int original = 42;

    // Serialize to JSON
    json_node << original;

    // Deserialize back to int
    int deserialized;
    json_node >> deserialized;

    // Check for equality
    if (original != deserialized) {
        std::cerr << "Int test failed. Expected: " << original << " but got: " << deserialized << std::endl;
        return 1; // Failure
    }
    return 0; // Success
}

int test_long()
{
    json::jnode_t json_node;
    long original = 1234567890L;

    // Serialize to JSON
    json_node << original;

    // Deserialize back to long
    long deserialized;
    json_node >> deserialized;

    // Check for equality
    if (original != deserialized) {
        std::cerr << "Long test failed. Expected: " << original << " but got: " << deserialized << std::endl;
        return 1; // Failure
    }
    return 0; // Success
}

int test_long_long()
{
#if __cplusplus >= 201103L
    json::jnode_t json_node;
    unsigned long long original = 9876543210123456789ULL;

    // Serialize to JSON
    json_node << original;

    // Deserialize back to long long
    unsigned long long deserialized;
    json_node >> deserialized;

    // Check for equality
    if (original != deserialized) {
        std::cerr << "Long long test failed. Expected: " << original << " but got: " << deserialized << std::endl;
        return 1; // Failure
    }
#endif
    return 0; // Success
}

int test_unsigned_long()
{
    json::jnode_t json_node;
    unsigned long original = 1234567890UL;

    // Serialize to JSON
    json_node << original;

    // Deserialize back to unsigned long
    unsigned long deserialized;
    json_node >> deserialized;

    // Check for equality
    if (original != deserialized) {
        std::cerr << "Unsigned long test failed. Expected: " << original << " but got: " << deserialized << std::endl;
        return 1; // Failure
    }
    return 0; // Success
}

int test_unsigned_long_long()
{
#if __cplusplus >= 201103L
    json::jnode_t json_node;
    unsigned long long original = 9876543210123456789ULL;

    // Serialize to JSON
    json_node << original;

    // Deserialize back to unsigned long long
    unsigned long long deserialized;
    json_node >> deserialized;

    // Check for equality
    if (original != deserialized) {
        std::cerr << "Unsigned long long test failed. Expected: " << original << " but got: " << deserialized << std::endl;
        return 1; // Failure
    }
#endif
    return 0; // Success
}

int test_float()
{
    json::jnode_t json_node;
    float original = 3.14f;

    // Serialize to JSON
    json_node << original;

    // Deserialize back to float
    float deserialized;
    json_node >> deserialized;

    // Check for equality
    if (std::abs(original - deserialized) > 1e-06) {
        std::cerr << "Float test failed. Expected: " << original << " but got: " << deserialized << std::endl;
        return 1; // Failure
    }
    return 0; // Success
}

int test_double()
{
    json::jnode_t json_node;
    double original = 2.71828;

    // Serialize to JSON
    json_node << original;

    // Deserialize back to double
    double deserialized;
    json_node >> deserialized;

    // Check for equality
    if (std::abs(original - deserialized) > 1e-09) {
        std::cerr << "Double test failed. Expected: " << original << " but got: " << deserialized << std::endl;
        return 1; // Failure
    }
    return 0; // Success
}

int test_char()
{
    json::jnode_t json_node;
    char original = 'A';

    // Serialize to JSON
    json_node << std::string(1, original); // Convert char to std::string for serialization

    // Deserialize back to char
    std::string deserialized_string;
    json_node >> deserialized_string;

    // Convert back to char
    char deserialized = deserialized_string[0];

    // Check for equality
    if (original != deserialized) {
        std::cerr << "Char test failed. Expected: " << original << " but got: " << deserialized << std::endl;
        return 1; // Failure
    }
    return 0; // Success
}

int test_bool()
{
    json::jnode_t json_node;
    bool original = true;

    // Serialize to JSON
    json_node << original;

    // Deserialize back to bool
    bool deserialized;
    json_node >> deserialized;

    // Check for equality
    if (original != deserialized) {
        std::cerr << "Bool test failed. Expected: " << original << " but got: " << deserialized << std::endl;
        return 1; // Failure
    }
    return 0; // Success
}

int test_string()
{
    json::jnode_t json_node;
    std::string original = "Hello, JSON!";

    // Serialize to JSON
    json_node << original;

    // Deserialize back to string
    std::string deserialized;
    json_node >> deserialized;

    // Check for equality
    if (original != deserialized) {
        std::cerr << "String test failed. Expected: " << original << " but got: " << deserialized << std::endl;
        return 1; // Failure
    }
    return 0; // Success
}

int test_enum()
{
    json::jnode_t json_node;
    Color original = GREEN; // Choose a value from the enum

    // Serialize to JSON (using the underlying integer value)
    json_node << static_cast<int>(original);

    // Deserialize back to enum (casting back from int)
    int deserialized_int;
    json_node >> deserialized_int;
    Color deserialized = static_cast<Color>(deserialized_int);

    // Check for equality
    if (original != deserialized) {
        std::cerr << "Enum test failed. Expected: " << original << " but got: " << deserialized << std::endl;
        return 1; // Failure
    }
    return 0; // Success
}

int test_pair()
{
    json::jnode_t json_node;
    std::pair<int, std::string> original = { 42, "Answer" };

    // Serialize to JSON
    json_node << original;

    // Deserialize back to pair
    std::pair<int, std::string> deserialized;
    json_node >> deserialized;

    // Check for equality
    if (original != deserialized) {
        std::cerr << "Pair test failed. Expected: (" << original.first << ", " << original.second << ")"
                  << " but got: (" << deserialized.first << ", " << deserialized.second << ")" << std::endl;
        return 1; // Failure
    }
    return 0; // Success
}

int test_tuple()
{
    json::jnode_t json_node;
    std::tuple<int, float, std::string> original = { 1, 2.5f, "Tuple" };

    // Serialize to JSON
    json_node << original;

    // Deserialize back to tuple
    std::tuple<int, float, std::string> deserialized;
    json_node >> deserialized;

    // Check for equality
    if (original != deserialized) {
        std::cerr << "Tuple test failed. Expected: (" << std::get<0>(original) << ", "
                  << std::get<1>(original) << ", " << std::get<2>(original) << ")"
                  << " but got: (" << std::get<0>(deserialized) << ", "
                  << std::get<1>(deserialized) << ", " << std::get<2>(deserialized) << ")" << std::endl;
        return 1; // Failure
    }
    return 0; // Success
}

int main()
{
    int result = 0;

    result += test_int();
    result += test_long();
    result += test_long_long();
    result += test_unsigned_long();
    result += test_unsigned_long_long();
    result += test_float();
    result += test_double();
    result += test_char();
    result += test_bool();
    result += test_string();
    result += test_enum();  // Enums
    result += test_pair();  // Pairs
    result += test_tuple(); // Tuples

    if (result == 0) {
        std::cout << "All tests passed!" << std::endl;
    } else {
        std::cout << result << " test(s) failed." << std::endl;
    }

    return result; // Return the number of failed tests
}