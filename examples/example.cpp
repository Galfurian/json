/// @file example.cpp
/// @author Enrico Fraccaroli (enry.frak@gmail.com)
/// @brief Contains a couple of examples with the json library.
///
/// @copyright (c) 2024 This file is distributed under the MIT License.
/// See LICENSE.md for details.
///

#include <json/json.hpp>

#include <iostream>
#include <sstream>
#include <tuple>

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
};

template <typename Type, std::size_t N, std::size_t Last>
struct tuple_printer {
    static void print(std::ostream &out, const Type &value)
    {
        out << std::get<N>(value) << ", ";
        tuple_printer<Type, N + 1, Last>::print(out, value);
    }
};

template <typename Type, std::size_t N>
struct tuple_printer<Type, N, N> {
    static void print(std::ostream &out, const Type &value) { out << std::get<N>(value); }
};

template <typename... Types>
std::ostream &operator<<(std::ostream &out, const std::tuple<Types...> &value)
{
    out << "(";
    tuple_printer<std::tuple<Types...>, 0, sizeof...(Types) - 1>::print(out, value);
    out << ")";
    return out;
}

int main(int, char *[])
{
    const char example[] = "{"
                           "   'p0': {"
                           "       'age': 24,"
                           "       'name': 'Alfred'"
                           "   }, 'p1': {"
                           "       'age': 45,"
                           "       'name': 'Juhan'"
                           "   },"
                           "   'v0': 1,"
                           "   'v1': false,"
                           "   'v2': 1.5,"
                           "   'v3': 1e+06,"
                           "   'v4': 1e-06,"
                           "   'v5': 'Hello world!',"
                           "   'v6': 97,"
                           "   'v7': 5,"
                           "   'v8': 7,"
                           "   'v9': {'real':0.75, 'imag':0.25},"
                           "   'v10': [-1, 1, 0.5]"
                           "}";
    // Prepare the recipients.
    Person p0;
    Person p1;
    int v0;
    bool v1;
    float v2;
    double v3;
    double v4;
    std::string v5;
    char v6;
    long v7;
#if __cplusplus >= 201103L
    long long v8;
#endif
    std::complex<double> v9;
    std::tuple<int, unsigned, float> v10;
    std::cout << "Parsing the json-formatted content...\n\n";
    // Prepare the example.
    json::jnode_t root = json::parser::parse(example);

    // Print the tree.
    std::cout << "This is the json tree we just built:\n";
    std::cout << root.to_string(true, 2) << "\n";

    std::cout << "\nExtracting the values from the json...\n\n";
    // Extract the values.
    root["p0"] >> p0;
    root["p1"] >> p1;
    root["v0"] >> v0;
    root["v1"] >> v1;
    root["v2"] >> v2;
    root["v3"] >> v3;
    root["v4"] >> v4;
    root["v5"] >> v5;
    root["v6"] >> v6;
    root["v7"] >> v7;
#if __cplusplus >= 201103L
    root["v8"] >> v8;
#endif
    root["v9"] >> v9;
    root["v10"] >> v10;

    // Print the values.
    std::cout << "These are the values we extracted:\n";
    std::cout << "    p0 : " << p0 << "\n";
    std::cout << "    p1 : " << p1 << "\n";
    std::cout << "    v0 : " << v0 << "\n";
    std::cout << "    v1 : " << v1 << "\n";
    std::cout << "    v2 : " << v2 << "\n";
    std::cout << "    v3 : " << v3 << "\n";
    std::cout << "    v4 : " << v4 << "\n";
    std::cout << "    v5 : " << v5 << "\n";
    std::cout << "    v6 : " << v6 << "\n";
    std::cout << "    v7 : " << v7 << "\n";
#if __cplusplus >= 201103L
    std::cout << "    v8 : " << v8 << "\n";
#endif
    std::cout << "    v9 : " << v9 << "\n";
    std::cout << "    v10 : " << v10 << "\n";
    return 0;
}