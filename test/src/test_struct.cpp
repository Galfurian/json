#include "json/json.hpp"
#include "person.hpp"

#include <iostream>
#include <sstream>

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

int main(int argc, char *argv[])
{
    // Define the values.
    Person out_p("Json", 47);
    // Prepare the recipients.
    Person in_p;
    // Prepare the output json tree.
    json::jnode_t out_root(json::JOBJECT);
    // Write the values.
    out_root["person"] << out_p;
    // Create the json string.
    std::string json = out_root.to_string(false, 0);
    // Parse the json string.
    json::jnode_t in_root = json::parser::parse(json);
    // Extract the values.
    in_root["person"] >> in_p;
    // Check equivalence.
    if (in_p != out_p) {
        std::cerr << "p : " << in_p << " != " << out_p << "\n";
        return 1;
    }
    return 0;
}