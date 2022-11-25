#include "json/json.hpp"

#include <iostream>
#include <sstream>

int main(int, char *[])
{
    // Define the values and their recipients.
    int in_v0, out_v0         = 1;
    bool in_v1, out_v1        = false;
    float in_v2, out_v2       = 1.5f;
    double in_v3, out_v3      = 1e06;
    double in_v4, out_v4      = 1e-06;
    std::string in_v5, out_v5 = "Hello world!";
    char in_v6, out_v6        = 'a';
    long in_v7, out_v7        = 5;
#if __cplusplus >= 201103L
    long long in_v8, out_v8   = 7;
#endif
    // Prepare the output json tree.
    json::jnode_t out_root(json::JOBJECT);
    // Write the values.
    out_root["v0"] << out_v0;
    out_root["v1"] << out_v1;
    out_root["v2"] << out_v2;
    out_root["v3"] << out_v3;
    out_root["v4"] << out_v4;
    out_root["v5"] << out_v5;
    out_root["v6"] << out_v6;
    out_root["v7"] << out_v7;
#if __cplusplus >= 201103L
    out_root["v8"] << out_v8;
#endif
    // Create the json string.
    std::string json = out_root.to_string(false, 0);
    // Parse the json string.
    json::jnode_t in_root = json::parser::parse(json);
    // Extract the values.
    in_root["v0"] >> in_v0;
    in_root["v1"] >> in_v1;
    in_root["v2"] >> in_v2;
    in_root["v3"] >> in_v3;
    in_root["v4"] >> in_v4;
    in_root["v5"] >> in_v5;
    in_root["v6"] >> in_v6;
    in_root["v7"] >> in_v7;
#if __cplusplus >= 201103L
    in_root["v8"] >> in_v8;
#endif
    // Check equivalence.
    if (in_v0 != out_v0) {
        std::cerr << "v0 : " << in_v0 << " != " << out_v0 << "\n";
        return 1;
    }
    if (in_v1 != out_v1) {
        std::cerr << "v1 : " << in_v1 << " != " << out_v1 << "\n";
        return 1;
    }
    if (in_v2 != out_v2) {
        std::cerr << "v2 : " << in_v2 << " != " << out_v2 << "\n";
        return 1;
    }
    if (in_v3 != out_v3) {
        std::cerr << "v3 : " << in_v3 << " != " << out_v3 << "\n";
        return 1;
    }
    if (in_v4 != out_v4) {
        std::cerr << "v4 : " << in_v4 << " != " << out_v4 << "\n";
        return 1;
    }
    if (in_v5 != out_v5) {
        std::cerr << "v5 : " << in_v5 << " != " << out_v5 << "\n";
        return 1;
    }
    if (in_v6 != out_v6) {
        std::cerr << "v6 : " << in_v6 << " != " << out_v6 << "\n";
        return 1;
    }
    if (in_v7 != out_v7) {
        std::cerr << "v7 : " << in_v7 << " != " << out_v7 << "\n";
        return 1;
    }
#if __cplusplus >= 201103L
    if (in_v8 != out_v8) {
        std::cerr << "v8 : " << in_v8 << " != " << out_v8 << "\n";
        return 1;
    }
#endif
    return 0;
}