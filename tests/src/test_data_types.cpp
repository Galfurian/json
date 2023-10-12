#include <json/json.hpp>

#include <iostream>
#include <sstream>

template <typename T>
int check_equivalence(const std::string &name, const T &v1, const T &v2)
{
    if (v1 != v2) {
        std::cerr << name << " : " << v1 << " != " << v2 << "\n";
        return 1;
    }
    return 0;
}

enum direction_t {
    north,
    south,
    east,
    west
};

JSON_DEFINE_OP_ENUM(direction_t)

int main(int, char *[])
{
    // ========================================================================
    // Define the values and their recipients.
    bool in_bool, out_bool               = false;
    char in_char, out_char               = 'a';
    unsigned char in_uchar, out_uchar    = 'z';
    short in_short, out_short            = -38;
    unsigned short in_ushort, out_ushort = +38;
    int in_int, out_int                  = -82;
    unsigned int in_uint, out_uint       = +82;
    long in_long, out_long               = -875;
    unsigned long in_ulong, out_ulong    = +875;
#if __cplusplus >= 201103L
    long long in_long_long, out_long_long            = -958234;
    unsigned long long in_ulong_long, out_ulong_long = +958234;
#endif
    float in_float, out_float         = 1.5f;
    double in_double_1, out_double_1  = 1e+06;
    double in_double_2, out_double_2  = 1e-06;
    std::string in_string, out_string = "Hello world!";
    direction_t in_enum, out_enum     = east;

    // ========================================================================
    // Prepare the output json tree.
    json::jnode_t out_root(json::JTYPE_OBJECT);

    // ========================================================================
    // Write the values.
    out_root["bool"] << out_bool;
    out_root["char"] << out_char;
    out_root["uchar"] << out_uchar;
    out_root["short"] << out_short;
    out_root["ushort"] << out_ushort;
    out_root["int"] << out_int;
    out_root["uint"] << out_uint;
    out_root["long"] << out_long;
    out_root["ulong"] << out_ulong;
#if __cplusplus >= 201103L
    out_root["long_long"] << out_long_long;
    out_root["ulong_long"] << out_ulong_long;
#endif
    out_root["float"] << out_float;
    out_root["double_1"] << out_double_1;
    out_root["double_2"] << out_double_2;
    out_root["string"] << out_string;
    out_root["enum"] << out_enum;

    // ========================================================================
    // Create the json string.
    std::string json_string = out_root.to_string(false, 0);

    // ========================================================================
    // Parse the json string.
    json::jnode_t in_root = json::parser::parse(json_string);

    // ========================================================================
    // Extract the values.
    in_root["bool"] >> in_bool;
    in_root["char"] >> in_char;
    in_root["uchar"] >> in_uchar;
    in_root["short"] >> in_short;
    in_root["ushort"] >> in_ushort;
    in_root["int"] >> in_int;
    in_root["uint"] >> in_uint;
    in_root["long"] >> in_long;
    in_root["ulong"] >> in_ulong;
#if __cplusplus >= 201103L
    in_root["long_long"] >> in_long_long;
    in_root["ulong_long"] >> in_ulong_long;
#endif
    in_root["float"] >> in_float;
    in_root["double_1"] >> in_double_1;
    in_root["double_2"] >> in_double_2;
    in_root["string"] >> in_string;
    in_root["enum"] >> in_enum;

    // ========================================================================
    // Check equivalence.
    return check_equivalence("int", in_int, out_int) ||
           check_equivalence("bool", in_bool, out_bool) ||
           check_equivalence("char", in_char, out_char) ||
           check_equivalence("uchar", in_uchar, out_uchar) ||
           check_equivalence("short", in_short, out_short) ||
           check_equivalence("ushort", in_ushort, out_ushort) ||
           check_equivalence("int", in_int, out_int) ||
           check_equivalence("uint", in_uint, out_uint) ||
           check_equivalence("long", in_long, out_long) ||
           check_equivalence("ulong", in_ulong, out_ulong) ||
#if __cplusplus >= 201103L
           check_equivalence("long_long", in_long_long, out_long_long) ||
           check_equivalence("ulong_long", in_ulong_long, out_ulong_long) ||
#endif
           check_equivalence("float", in_float, out_float) ||
           check_equivalence("double_1", in_double_1, out_double_1) ||
           check_equivalence("double_2", in_double_2, out_double_2) ||
           check_equivalence("string", in_string, out_string) ||
           check_equivalence("enum", in_enum, out_enum);
}
