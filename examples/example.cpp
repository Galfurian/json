#include "json/json.hpp"

#include <iostream>
#include <sstream>

struct Person {
    std::string name;
    unsigned age;

    Person()
        : name(),
          age()
    {
    }

    Person(std::string _name, unsigned _age)
        : name(_name),
          age(_age)
    {
    }

    friend inline std::ostream &operator<<(std::ostream &lhs, const Person &rhs)
    {
        lhs << "[" << rhs.name << ", " << rhs.age << "]";
        return lhs;
    }
};

namespace json
{
template <>
json::jnode_t &operator<<(json::jnode_t &lhs, const Person &rhs)
{
    lhs.set_type(json::JOBJECT);
    lhs["name"] << rhs.name;
    lhs["age"] << rhs.age;
    return lhs;
}

template <>
const json::jnode_t &operator>>(const json::jnode_t &lhs, Person &rhs)
{
    lhs["name"] >> rhs.name;
    lhs["age"] >> rhs.age;
    return lhs;
}
} // namespace json

int main(int, char *[])
{
    const char example[] = "{'p0': {'age': 24,'name': 'Alfred'},'p1': {'age': 45,'name': 'Juhan'},'v0': 1,'v1': false,'v2': 1.5,'v3': 1e+06,'v4': 1e-06,'v5': 'Hello world!','v6': 97,'v7': 5,'v8': 7}";
    // Prepare the recipients.
    int v0;
    bool v1;
    float v2;
    double v3;
    double v4;
    std::string v5;
    char v6;
    long v7;
    long long v8;
    Person p0;
    Person p1;
    // Prepare the example.
    json::jnode_t root = json::parser::parse(example);
    // Print the tree.
    std::cout << root.to_string(true, 2) << "\n";
    // Extract the values.
    root["v0"] >> v0;
    root["v1"] >> v1;
    root["v2"] >> v2;
    root["v3"] >> v3;
    root["v4"] >> v4;
    root["v5"] >> v5;
    root["v6"] >> v6;
    root["v7"] >> v7;
    root["v8"] >> v8;
    root["p0"] >> p0;
    root["p1"] >> p1;
    // Print the values.
    std::cout << "v0 : " << v0 << "\n";
    std::cout << "v1 : " << v1 << "\n";
    std::cout << "v2 : " << v2 << "\n";
    std::cout << "v3 : " << v3 << "\n";
    std::cout << "v4 : " << v4 << "\n";
    std::cout << "v5 : " << v5 << "\n";
    std::cout << "v6 : " << v6 << "\n";
    std::cout << "v7 : " << v7 << "\n";
    std::cout << "v8 : " << v8 << "\n";
    std::cout << "p0 : " << p0 << "\n";
    std::cout << "p1 : " << p1 << "\n";
    return 0;
}