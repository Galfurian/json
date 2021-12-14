#include <iostream>

#include "json/json.hpp"

// ============================================================================
// == Support
// ============================================================================

template <typename T>
struct TypeName {
    static std::string Get();
};

template <>
struct TypeName<bool> {
    static std::string Get()
    {
        return "bool";
    }
};

template <>
struct TypeName<int> {
    static std::string Get()
    {
        return "int";
    }
};

template <>
struct TypeName<double> {
    static std::string Get()
    {
        return "double";
    }
};

template <>
struct TypeName<unsigned> {
    static std::string Get()
    {
        return "unsigned";
    }
};

// ============================================================================
// == CLASSES
// ============================================================================

class Entity {
public:
    virtual ~Entity() = default;
};

template <typename T>
class Variable : public Entity {
public:
    std::string name;
    T value;

    Variable()
        : name(), value()
    {
        // Nothing to do.
    }

    Variable(const std::string &_name, T _value)
        : name(_name),
          value(_value)
    {
        // Nothing to do.
    }

    ~Variable() override = default;

    static inline std::string get_typename()
    {
        return "Variable<" + TypeName<T>::Get() + ">";
    }
};

class Node : public Entity {
public:
    std::string name;

    Node()
        : name()
    {
        // Nothing to do.
    }

    Node(const std::string &_name)
        : name(_name)
    {
        // Nothing to do.
    }

    ~Node() override = default;

    static inline std::string get_typename()
    {
        return "Node";
    }
};

class Component : public Entity {
public:
    std::string name;
    std::vector<Entity *> variables;
    std::vector<Node *> nodes;

    Component()
        : name(),
          variables(),
          nodes()
    {
        // Nothing to do.
    }

    Component(const std::string &_name, const std::vector<Entity *> &_variables, const std::vector<Node *> &_nodes)
        : name(_name),
          variables(_variables),
          nodes(_nodes)
    {
        // Nothing to do.
    }

    ~Component() override
    {
        for (size_t i = 0; i < variables.size(); ++i)
            delete variables[i];
        for (size_t i = 0; i < nodes.size(); ++i)
            delete nodes[i];
    }

    static inline std::string get_typename()
    {
        return "Component";
    }
};

class Subckt : public Entity {
public:
    std::vector<Entity *> entities;

    Subckt()
        : entities()
    {
    }

    Subckt(const std::vector<Entity *> &_entities)
        : entities(_entities)
    {
    }

    ~Subckt() override
    {
        for (size_t i = 0; i < entities.size(); ++i)
            delete entities[i];
    }

    static inline std::string get_typename()
    {
        return "Subckt";
    }
};

// ============================================================================
// == JSON Streamers
// ============================================================================

template <typename Base, typename Derived>
static inline bool __try_to_read(const json::jnode_t &lhs, Base *&rhs)
{
    Derived *ptr = nullptr;
    (lhs >> ptr);
    if (ptr) {
        rhs = ptr;
        return true;
    }
    return false;
}

template <typename Base, typename Derived>
static inline bool __try_to_write(json::jnode_t &lhs, Base *const &rhs)
{
    auto ptr = dynamic_cast<Derived *>(rhs);
    if (ptr) {
        (lhs << ptr);
        return true;
    }
    return false;
}

template <>
json::jnode_t &json::operator<<<Subckt *>(json::jnode_t &lhs, Subckt *const &rhs)
{
    // Set the type to object.
    lhs.set_type(json::JOBJECT);
    // Write the type.
    lhs["type"] << Subckt::get_typename();
    // Write the fields.
    lhs["entities"] << rhs->entities;
    return lhs;
}

template <>
json::jnode_t &json::operator<<<Component *>(json::jnode_t &lhs, Component *const &rhs)
{
    // Set the type to object.
    lhs.set_type(json::JOBJECT);
    // Write the type.
    lhs["type"] << Component::get_typename();
    // Write the fields.
    lhs["name"] << rhs->name;
    lhs["variables"] << rhs->variables;
    lhs["nodes"] << rhs->nodes;
    return lhs;
}

template <>
json::jnode_t &json::operator<<<Node *>(json::jnode_t &lhs, Node *const &rhs)
{
    // Set the type to object.
    lhs.set_type(json::JOBJECT);
    // Write the type.
    lhs["type"] << Node::get_typename();
    // Write the fields.
    lhs["name"] << rhs->name;
    return lhs;
}

template <typename T>
json::jnode_t &json::operator<<(json::jnode_t &lhs, Variable<T> *const &rhs)
{
    // Set the type to object.
    lhs.set_type(json::JOBJECT);
    // Write the type.
    lhs["type"] << Variable<T>::get_typename();
    // Write the fields.
    lhs["name"] << rhs->name;
    lhs["value"] << rhs->value;
    return lhs;
}

template <>
json::jnode_t &json::operator<<<Entity *>(json::jnode_t &lhs, Entity *const &rhs)
{
    if (__try_to_write<Entity, Subckt>(lhs, rhs))
        return lhs;
    if (__try_to_write<Entity, Component>(lhs, rhs))
        return lhs;
    if (__try_to_write<Entity, Node>(lhs, rhs))
        return lhs;
    if (__try_to_write<Entity, Variable<bool>>(lhs, rhs))
        return lhs;
    if (__try_to_write<Entity, Variable<int>>(lhs, rhs))
        return lhs;
    if (__try_to_write<Entity, Variable<unsigned>>(lhs, rhs))
        return lhs;
    if (__try_to_write<Entity, Variable<double>>(lhs, rhs))
        return lhs;
    return lhs;
}

template <>
const json::jnode_t &json::operator>><Subckt *>(const json::jnode_t &lhs, Subckt *&rhs)
{
    if (lhs["type"].get_value() == Subckt::get_typename()) {
        if (rhs == nullptr)
            rhs = new Subckt();
        lhs["entities"] >> rhs->entities;
    }
    return lhs;
}

template <>
const json::jnode_t &json::operator>><Component *>(const json::jnode_t &lhs, Component *&rhs)
{
    if (lhs["type"].get_value() == Component::get_typename()) {
        if (rhs == nullptr)
            rhs = new Component();
        lhs["name"] >> rhs->name;
        lhs["variables"] >> rhs->variables;
        lhs["nodes"] >> rhs->nodes;
    }
    return lhs;
}

template <>
const json::jnode_t &json::operator>><Node *>(const json::jnode_t &lhs, Node *&rhs)
{
    if (lhs["type"].get_value() == Node::get_typename()) {
        if (rhs == nullptr)
            rhs = new Node();
        lhs["name"] >> rhs->name;
    }
    return lhs;
}

template <typename T>
const json::jnode_t &json::operator>>(const json::jnode_t &lhs, Variable<T> *&rhs)
{
    if (lhs["type"].get_value() == Variable<T>::get_typename()) {
        if (rhs == nullptr)
            rhs = new Variable<T>();
        lhs["name"] >> rhs->name;
        lhs["value"] >> rhs->value;
    }
    return lhs;
}

template <>
const json::jnode_t &json::operator>><Entity *>(const json::jnode_t &lhs, Entity *&rhs)
{
    if (__try_to_read<Entity, Subckt>(lhs, rhs))
        return lhs;
    if (__try_to_read<Entity, Component>(lhs, rhs))
        return lhs;
    if (__try_to_read<Entity, Node>(lhs, rhs))
        return lhs;
    if (__try_to_read<Entity, Variable<bool>>(lhs, rhs))
        return lhs;
    if (__try_to_read<Entity, Variable<int>>(lhs, rhs))
        return lhs;
    if (__try_to_read<Entity, Variable<unsigned>>(lhs, rhs))
        return lhs;
    if (__try_to_read<Entity, Variable<double>>(lhs, rhs))
        return lhs;
    return lhs;
}

// ============================================================================
// == MAIN
// ============================================================================
int main(int argc, char *argv[])
{
    json::jnode_t root0(json::JOBJECT);
    {
        auto subckt = new Subckt(
            {
                new Node("gn1"),
                new Node("gn2"),
                new Variable<int>("A", 1),
                new Component(
                    "res0",
                    {
                        new Variable<int>("B", 1),
                    },
                    {
                        new Node("n1"),
                        new Node("n2"),
                    }),
            });
        root0 << subckt;
        delete subckt;
    }
    json::jnode_t root1(json::JOBJECT);
    {
        Subckt *subckt = nullptr;
        root0 >> subckt;
        root1 << subckt;
        delete subckt;
    }
    std::cout << root0.to_string(true, 4) << "\n";
    std::cout << root1.to_string(true, 4) << "\n";
    return 0;
}