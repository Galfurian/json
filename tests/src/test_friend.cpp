#include <json/json.hpp>

#include <iostream>
#include <sstream>

class Animal {
private:
    std::string name;

public:
    Animal()
        : name()
    {
    }

    Animal(std::string _name)
        : name(_name)
    {
    }

    friend inline bool operator==(const Animal &lhs, const Animal &rhs)
    {
        return lhs.name == rhs.name;
    }

    friend inline bool operator!=(const Animal &lhs, const Animal &rhs)
    {
        return lhs.name != rhs.name;
    }

    friend inline std::ostream &operator<<(std::ostream &lhs, const Animal &rhs)
    {
        lhs << "[" << rhs.name << "]";
        return lhs;
    }

    friend json::jnode_t &operator<<(json::jnode_t &lhs, const Animal &rhs)
    {
        lhs.set_type(json::JOBJECT);
        lhs["name"] << rhs.name;
        return lhs;
    }

    friend const json::jnode_t &operator>>(const json::jnode_t &lhs, Animal &rhs)
    {
        lhs["name"] >> rhs.name;
        return lhs;
    }
};

int main(int, char *[])
{
    // Define the values.
    Animal cat_out("cat");
    // Prepare the recipients.
    Animal cat_in;
    // Prepare the output json tree.
    json::jnode_t out_root(json::JOBJECT);
    // Write the values.
    out_root["Animal"] << cat_out;
    // Create the json string.
    std::string json = out_root.to_string(false, 0);
    // Parse the json string.
    json::jnode_t in_root = json::parser::parse(json);
    // Extract the values.
    in_root["Animal"] >> cat_in;
    // Check equivalence.
    if (cat_in != cat_out) {
        std::cerr << "Animal : " << cat_in << " != " << cat_out << "\n";
        return 1;
    }
    return 0;
}