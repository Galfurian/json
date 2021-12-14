/// @file typename.hpp
/// @author Enrico Fraccaroli (enry.frak@gmail.com)
/// @brief
/// @version 0.1
/// @date 2021-12-14
///
/// @copyright Copyright (c) 2021
///

#pragma once

#include <string>

template <typename T>
struct TypeName {
    static std::string get();
};

template <>
struct TypeName<bool> {
    static std::string get()
    {
        return "bool";
    }
};

template <>
struct TypeName<char> {
    static std::string get()
    {
        return "char";
    }
};

template <>
struct TypeName<unsigned char> {
    static std::string get()
    {
        return "unsigned char";
    }
};

template <>
struct TypeName<short> {
    static std::string get()
    {
        return "short";
    }
};

template <>
struct TypeName<unsigned short> {
    static std::string get()
    {
        return "unsigned short";
    }
};

template <>
struct TypeName<int> {
    static std::string get()
    {
        return "int";
    }
};

template <>
struct TypeName<unsigned int> {
    static std::string get()
    {
        return "unsigned int";
    }
};

template <>
struct TypeName<long> {
    static std::string get()
    {
        return "long";
    }
};

template <>
struct TypeName<unsigned long> {
    static std::string get()
    {
        return "unsigned long";
    }
};

template <>
struct TypeName<float> {
    static std::string get()
    {
        return "float";
    }
};

template <>
struct TypeName<double> {
    static std::string get()
    {
        return "double";
    }
};

template <>
struct TypeName<long double> {
    static std::string get()
    {
        return "long double";
    }
};
