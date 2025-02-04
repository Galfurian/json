/// @file test_container.cpp
/// @author Enrico Fraccaroli (enry.frak@gmail.com)
/// @brief Tests compatibility of the json library with containers.
///
/// @copyright (c) 2024 This file is distributed under the MIT License.
/// See LICENSE.md for details.
///

#include <json/json.hpp>

#include <array>
#include <deque>
#include <iostream>
#include <sstream>
#include <unordered_map>

enum Color { RED, GREEN, BLUE };

// Functions to convert between Color and string
static inline std::string color_to_string(Color color)
{
    switch (color) {
    case Color::RED:
        return "RED";
    case Color::GREEN:
        return "GREEN";
    case Color::BLUE:
        return "BLUE";
    default:
        return "UNKNOWN";
    }
}

int test_vector()
{
    json::jnode_t json_node;
    std::vector<int> original = {1, 2, 3, 4, 5};

    // Serialize to JSON
    json_node << original;

    // Deserialize back to vector
    std::vector<int> deserialized;
    json_node >> deserialized;

    // Check for equality
    if (original != deserialized) {
        std::cerr << "Vector test failed. Expected: ";
        for (const auto &item : original) {
            std::cerr << item << " ";
        }
        std::cerr << "but got: ";
        for (const auto &item : deserialized) {
            std::cerr << item << " ";
        }
        std::cerr << std::endl;
        return 1; // Failure
    }
    return 0; // Success
}

int test_list()
{
    json::jnode_t json_node;
    std::list<std::string> original = {"apple", "banana", "cherry"};

    // Serialize to JSON
    json_node << original;

    // Deserialize back to list
    std::list<std::string> deserialized;
    json_node >> deserialized;

    // Check for equality
    if (original != deserialized) {
        std::cerr << "List test failed. Expected: ";
        for (const auto &item : original) {
            std::cerr << item << " ";
        }
        std::cerr << "but got: ";
        for (const auto &item : deserialized) {
            std::cerr << item << " ";
        }
        std::cerr << std::endl;
        return 1; // Failure
    }
    return 0; // Success
}

int test_set()
{
    json::jnode_t json_node;
    std::set<std::string> original = {"one", "two", "three"};

    // Serialize to JSON
    json_node << original;

    // Deserialize back to set
    std::set<std::string> deserialized;
    json_node >> deserialized;

    // Check for equality
    if (original != deserialized) {
        std::cerr << "Set test failed. Expected: ";
        for (const auto &item : original) {
            std::cerr << item << " ";
        }
        std::cerr << "but got: ";
        for (const auto &item : deserialized) {
            std::cerr << item << " ";
        }
        std::cerr << std::endl;
        return 1; // Failure
    }
    return 0; // Success
}

int test_map_string()
{
    json::jnode_t json_node;
    std::map<std::string, int> original = {{"Alice", 30}, {"Bob", 25}, {"Charlie", 35}};

    // Serialize to JSON
    json_node << original;

    // Deserialize back to map
    std::map<std::string, int> deserialized;
    json_node >> deserialized;

    // Check for equality
    if (original != deserialized) {
        std::cerr << "Map test failed. Expected: ";
        for (const auto &pair : original) {
            std::cerr << pair.first << ": " << pair.second << " ";
        }
        std::cerr << "but got: ";
        for (const auto &pair : deserialized) {
            std::cerr << pair.first << ": " << pair.second << " ";
        }
        std::cerr << std::endl;
        return 1; // Failure
    }
    return 0; // Success
}

int test_map_enum()
{
    json::jnode_t json_node;
    std::map<Color, int> original = {{Color::RED, 30}, {Color::GREEN, 25}, {Color::BLUE, 35}}, deserialized;

    // Serialize to JSON
    json_node << original;

    // Deserialize back to map
    json_node >> deserialized;

    // Check for equality
    if (original != deserialized) {
        std::cerr << "Map test failed. Expected: ";
        for (const auto &pair : original) {
            std::cerr << pair.first << ": " << pair.second << " ";
        }
        std::cerr << "but got: ";
        for (const auto &pair : deserialized) {
            std::cerr << pair.first << ": " << pair.second << " ";
        }
        std::cerr << std::endl;
        return 1; // Failure
    }
    return 0; // Success
}

// Test function for std::map with enum keys
int test_enum_map()
{
    json::jnode_t json_node;
    std::map<Color, int> original = {{Color::RED, 30}, {Color::GREEN, 25}, {Color::BLUE, 35}};

    // Serialize to JSON.
    json_node << original;

    // Deserialize back to map.
    std::map<Color, int> deserialized;
    json_node >> deserialized;

    // Check for equality
    if (original != deserialized) {
        std::cerr << "Enum Map test failed. Expected: ";
        for (const auto &pair : original) {
            std::cerr << color_to_string(pair.first) << ": " << pair.second << " ";
        }
        std::cerr << "but got: ";
        for (const auto &pair : deserialized) {
            std::cerr << color_to_string(pair.first) << ": " << pair.second << " ";
        }
        std::cerr << std::endl;
        return 1; // Failure
    }
    return 0; // Success
}

int test_array()
{
    json::jnode_t json_node;
    std::array<int, 3> original = {10, 20, 30}, deserialized;

    // Serialize to JSON
    json_node << original;

    // Deserialize back to array
    json_node >> deserialized;

    // Check for equality
    if (original != deserialized) {
        std::cerr << "Array test failed. Expected: ";
        for (const auto &item : original) {
            std::cerr << item << " ";
        }
        std::cerr << "but got: ";
        for (const auto &item : deserialized) {
            std::cerr << item << " ";
        }
        std::cerr << std::endl;
        return 1; // Failure
    }
    return 0; // Success
}

int test_deque()
{
    json::jnode_t json_node;
    std::deque<int> original = {1, 2, 3, 4, 5}, deserialized;

    // Serialize to JSON
    json_node << original;

    // Deserialize back to deque
    json_node >> deserialized;

    // Check for equality
    if (original != deserialized) {
        std::cerr << "Deque test failed. Expected: ";
        for (const auto &item : original) {
            std::cerr << item << " ";
        }
        std::cerr << "but got: ";
        for (const auto &item : deserialized) {
            std::cerr << item << " ";
        }
        std::cerr << std::endl;
        return 1; // Failure
    }
    return 0; // Success
}

int test_unordered_map_string()
{
    json::jnode_t json_node;
    std::unordered_map<std::string, int> original = {{"Alice", 30}, {"Bob", 25}, {"Charlie", 35}}, deserialized;

    // Serialize to JSON
    json_node << original;

    // Deserialize back to unordered_map
    json_node >> deserialized;

    // Check for equality
    if (original != deserialized) {
        std::cerr << "Unordered map test failed. Expected: ";
        for (const auto &pair : original) {
            std::cerr << pair.first << ": " << pair.second << " ";
        }
        std::cerr << "but got: ";
        for (const auto &pair : deserialized) {
            std::cerr << pair.first << ": " << pair.second << " ";
        }
        std::cerr << std::endl;
        return 1; // Failure
    }
    return 0; // Success
}

int test_unordered_map_enum()
{
    json::jnode_t json_node;
    std::unordered_map<Color, int> original = {{Color::RED, 1}, {Color::GREEN, 2}}, deserialized;

    // Serialize to JSON
    json_node << original;

    // Deserialize back to unordered_map
    json_node >> deserialized;

    // Check for equality
    if (original != deserialized) {
        std::cerr << "Unordered map test failed. Expected: ";
        for (const auto &pair : original) {
            std::cerr << pair.first << ": " << pair.second << " ";
        }
        std::cerr << "but got: ";
        for (const auto &pair : deserialized) {
            std::cerr << pair.first << ": " << pair.second << " ";
        }
        std::cerr << std::endl;
        return 1; // Failure
    }
    return 0; // Success
}

int test_bitset()
{
    json::jnode_t json_node;
    std::bitset<8> original("10101010"), deserialized;

    // Serialize to JSON
    json_node << original;

    // Deserialize back to bitset
    json_node >> deserialized;

    // Check for equality
    if (original != deserialized) {
        std::cerr << "Bitset test failed. Expected: " << original.to_string()
                  << " but got: " << deserialized.to_string() << std::endl;
        return 1; // Failure
    }
    return 0; // Success
}

int main(int, char *[])
{
    int result = 0;

    result += test_vector();
    result += test_list();
    result += test_set();
    result += test_array();
    result += test_deque();
    result += test_map_string();
    result += test_map_enum();
    result += test_unordered_map_string();
    result += test_unordered_map_enum();
    result += test_bitset();

    if (result == 0) {
        std::cout << "All tests passed!" << std::endl;
    } else {
        std::cout << result << " test(s) failed." << std::endl;
    }

    // Return the number of failed tests.
    return result;
}
