/// @file person.hpp
/// @author Enrico Fraccaroli (enry.frak@gmail.com)
/// @brief

#pragma once

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

    friend inline bool operator==(const Person &lhs, const Person &rhs)
    {
        if (lhs.name != rhs.name)
            return false;
        if (lhs.age != rhs.age)
            return false;
        return true;
    }

    friend inline bool operator!=(const Person &lhs, const Person &rhs)
    {
        if (lhs.name != rhs.name)
            return true;
        if (lhs.age != rhs.age)
            return true;
        return false;
    }

    friend inline std::ostream &operator<<(std::ostream &lhs, const Person &rhs)
    {
        lhs << "[" << rhs.name << ", " << rhs.age << "]";
        return lhs;
    }
};
