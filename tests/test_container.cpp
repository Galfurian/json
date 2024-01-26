/// @file test_container.cpp
/// @author Enrico Fraccaroli (enry.frak@gmail.com)
/// @brief Tests compatibility of the json library with containers.
/// 
/// @copyright (c) 2024 This file is distributed under the MIT License.
/// See LICENSE.md for details.
/// 

#include <iostream>
#include <json/json.hpp>
#include <sstream>

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

int main(int, char *[])
{
    // Define the values.
    std::vector<Person> out_ps;
    out_ps.push_back(Person("Json", 47));
    out_ps.push_back(Person("Terry", 23));
    // Prepare the recipients.
    std::vector<Person> in_ps;
    // Prepare the output json tree.
    json::jnode_t out_root(json::JTYPE_OBJECT);
    // Write the values.
    out_root["persons"] << out_ps;
    // Create the json string.
    std::string json = out_root.to_string(false, 0);
    // Parse the json string.
    json::jnode_t in_root = json::parser::parse(json);
    // Extract the values.
    in_root["persons"] >> in_ps;
    // Check equivalence.
    if (in_ps.size() != out_ps.size()) {
        std::cerr << "size : " << in_ps.size() << " != " << out_ps.size() << "\n";
        return 1;
    }
    for (unsigned i = 0; i < in_ps.size(); ++i) {
        if (in_ps[i] != out_ps[i]) {
            std::cerr << "p[" << i << "] : " << in_ps[i] << " != " << out_ps[i] << "\n";
            return 1;
        }
    }
    return 0;
}
