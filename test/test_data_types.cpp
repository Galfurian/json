#include "json/json.hpp"

#include <iostream>
#include <sstream>

int main(int argc, char *argv[])
{
    json::jnode_t out_root(json::JOBJECT);
    {
        // Write the values.
        out_root["int"] << 1;
        out_root["bool"] << false;
        out_root["float"] << 1.5f;
        out_root["double1"] << 1e06;
        out_root["double2"] << 1e-06;
        out_root["string"] << "Hello world!";
        out_root["char"] << 'a';
        // Show the result.
        std::cout << out_root.to_string(true, 4) << "\n";
    }
    // Create the json string.
    std::string json = out_root.to_string(false, 0);
    // Parse the json string.
    json::jnode_t in_root = json::parser::parse(json);
    {
        // Prepare the recipients.
        int v0;
        bool v1;
        float v2;
        double v3;
        double v4;
        std::string v5;
        char v6;
        // Extract the values.
        in_root["int"] >> v0;
        in_root["bool"] >> v1;
        in_root["float"] >> v2;
        in_root["double1"] >> v3;
        in_root["double2"] >> v4;
        in_root["string"] >> v5;
        in_root["char"] >> v6;
        // Show the result.
        std::cout << "int     : " << v0 << "\n";
        std::cout << "bool    : " << v1 << "\n";
        std::cout << "float   : " << v2 << "\n";
        std::cout << "double1 : " << v3 << "\n";
        std::cout << "double2 : " << v4 << "\n";
        std::cout << "string  : " << v5 << "\n";
        std::cout << "char    : " << v6 << "\n";
    }
    return 0;
}