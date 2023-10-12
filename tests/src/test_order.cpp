#include <json/json.hpp>
#include <iostream>
#include <sstream>

bool check_property_name(const json::jnode_t::property_map_t::const_iterator &it, const std::string &name)
{
    if (it->first != name) {
        std::cerr << it->first << " != " << name << "\n";
        return false;
    }
    return true;
}

int main(int, char *[])
{
    // Prepare the recipients.
    int v0   = 1;
    bool v1  = false;
    float v2 = 3.14f;

    // Prepare the example.
    json::jnode_t root(json::JTYPE_OBJECT);

    // Extract the values.
    root["v0"] << v0;
    root["v2"] << v2;
    root["v1"] << v1;

    // Check the order.
    json::jnode_t::property_map_t::const_iterator it = root.pbegin();
    if (!check_property_name(it, "v0"))
        return 1;
    if (!it->second.is_number())
        return 1;
    // Move to the next.
    ++it;
    if (!check_property_name(it, "v2"))
        return 1;
    if (!it->second.is_number())
        return 1;
    // Move to the next.
    ++it;
    if (!check_property_name(it, "v1"))
        return 1;
    if (!it->second.is_bool())
        return 1;
    return 0;
}
