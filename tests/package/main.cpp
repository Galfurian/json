/// @file main.cpp
/// @brief Minimal consumer of the installed package, built by the Install CI job.

#include <iostream>
#include <json/json.hpp>

int main()
{
    json::jnode_t root = json::parser::parse("{a: [0x10, 2]}");
    std::cout << root.to_string(false) << "\n";
    return (root["a"][0].as_number<int>() == 16) ? 0 : 1;
}
