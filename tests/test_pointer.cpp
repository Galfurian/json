/// @file test_pointer.cpp
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
        : name(_name)
        , age(_age)
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

    friend const json::jnode_t &operator>>(const json::jnode_t &lhs, Person *&rhs)
    {
        if (!rhs)
            rhs = new Person();
        lhs >> (*rhs);
        return lhs;
    }

    friend const json::jnode_t &operator>>(const json::jnode_t &lhs, std::shared_ptr<Person> &rhs)
    {
        if (!rhs)
            rhs = std::make_shared<Person>();
        lhs >> (*rhs);
        return lhs;
    }
};

int test_pointer()
{
    // Define the values.
    std::vector<Person *> out;
    out.push_back(new Person("Json", 47));
    out.push_back(new Person("Terry", 23));
    // Prepare the recipients.
    std::vector<Person *> in;
    // Prepare the output json tree.
    json::jnode_t out_root(json::JTYPE_OBJECT);
    // Write the values.
    out_root["people"] << out;
    // Create the json string.
    std::string json      = out_root.to_string(false, 0);
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
        if (*in[i] != *out[i]) {
            std::cerr << "p[" << i << "] : " << in[i] << " != " << out[i] << "\n";
            return 1;
        }
    }
    return 0;
}

int test_smart_pointer()
{
    // Define the values.
    std::vector<std::shared_ptr<Person>> out;
    out.push_back(std::make_shared<Person>("Json", 47));
    out.push_back(std::make_shared<Person>("Terry", 23));
    // Prepare the recipients.
    std::vector<std::shared_ptr<Person>> in;
    // Prepare the output json tree.
    json::jnode_t out_root(json::JTYPE_OBJECT);
    // Write the values.
    out_root["people"] << out;
    // Create the json string.
    std::string json      = out_root.to_string(false, 0);
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
        if (*in[i] != *out[i]) {
            std::cerr << "p[" << i << "] : " << in[i] << " != " << out[i] << "\n";
            return 1;
        }
    }
    return 0;
}

int main(int, char *[])
{
    if (test_pointer()) {
        return 1;
    }
    if (test_smart_pointer()) {
        return 1;
    }
    return 0;
}
