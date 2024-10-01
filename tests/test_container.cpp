/// @file test_container.cpp
/// @author Enrico Fraccaroli (enry.frak@gmail.com)
/// @brief Tests compatibility of the json library with containers.
///
/// @copyright (c) 2024 This file is distributed under the MIT License.
/// See LICENSE.md for details.
///

#include <json/json.hpp>
#include <iostream>
#include <sstream>
#include <array>

struct Person {
    std::string name;
    unsigned age;

    Person() = default;

    Person(std::string _name, unsigned _age)
        : name(_name),
          age(_age)
    {
    }

    inline bool operator==(const Person &rhs)
    {
        if (name != rhs.name) {
            return false;
        }
        if (age != rhs.age) {
            return false;
        }
        return true;
    }

    inline bool operator!=(const Person &rhs)
    {
        if (name != rhs.name) {
            return true;
        }
        if (age != rhs.age) {
            return true;
        }
        return false;
    }

    friend inline std::ostream &operator<<(std::ostream &lhs, const Person &rhs)
    {
        lhs << "[" << rhs.name << ", " << rhs.age << "]";
        return lhs;
    }

    friend json::jnode_t &operator<<(json::jnode_t &lhs, const Person &rhs)
    {
        lhs.set_type(json::JTYPE_OBJECT);
        lhs["name"] << rhs.name;
        lhs["age"] << rhs.age;
        return lhs;
    }

    friend const json::jnode_t &operator>>(const json::jnode_t &lhs, Person &rhs)
    {
        lhs["name"] >> rhs.name;
        lhs["age"] >> rhs.age;
        return lhs;
    }
};

int test_vector()
{
    // Define the values.
    std::vector<Person> out;
    out.push_back(Person("Json", 47));
    out.push_back(Person("Terry", 23));
    // Prepare the recipients.
    std::vector<Person> in;
    // Prepare the output json tree.
    json::jnode_t out_root(json::JTYPE_OBJECT);
    // Write the values.
    out_root["people"] << out;
    // Create the json string.
    std::string json = out_root.to_string(false, 0);
    // Parse the json string.
    json::jnode_t in_root = json::parser::parse(json);
    // Extract the values.
    in_root["people"] >> in;
    // Check equivalence.
    if (in.size() != out.size()) {
        std::cerr << "size : " << in.size() << " != " << out.size() << "\n";
        return 1;
    }
    for (unsigned i = 0; i < in.size(); ++i) {
        if (in[i] != out[i]) {
            std::cerr << "p[" << i << "] : " << in[i] << " != " << out[i] << "\n";
            return 1;
        }
    }
    return 0;
}

int test_array()
{
    // Define the values.
    std::array<Person, 2UL> out;
    out[0] = Person("Json", 47);
    out[1] = Person("Terry", 23);
    // Prepare the recipients.
    std::array<Person, 2UL> in;
    // Prepare the output json tree.
    json::jnode_t out_root(json::JTYPE_OBJECT);
    // Write the values.
    out_root["people"] << out;
    // Create the json string.
    std::string json = out_root.to_string(false, 0);
    // Parse the json string.
    json::jnode_t in_root = json::parser::parse(json);
    // Extract the values.
    in_root["people"] >> in;
    // Check equivalence.
    for (unsigned i = 0; i < in.size(); ++i) {
        if (in[i] != out[i]) {
            std::cerr << "p[" << i << "] : " << in[i] << " != " << out[i] << "\n";
            return 1;
        }
    }
    return 0;
}

int main(int, char *[])
{
    if (test_vector()) {
        return 1;
    }
    if (test_array()) {
        return 1;
    }
    return 0;
}
