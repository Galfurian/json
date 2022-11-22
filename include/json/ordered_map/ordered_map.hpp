/// @file ordered_map.hpp
/// @author Enrico Fraccaroli (enry.frak@gmail.com)
/// @brief

#pragma once

#include <map>
#include <list>

/// @brief This namespace contains the main table class.
namespace ordered_map
{

template <typename Key, typename Value>
class ordered_map_t {
public:
    typedef std::pair<Key, Value> table_node_t;
    typedef typename std::list<table_node_t>::iterator iterator;
    typedef typename std::list<table_node_t>::const_iterator const_iterator;
    typedef std::pair<Value, iterator> table_ref_t;

    void clear()
    {
        list.clear();
        data.clear();
    }

    std::size_t size() const
    {
        return list.size();
    }

    ordered_map_t &push(const Key &key, const Value &value)
    {
        list.emplace_back(key, value);
        data.emplace(key, table_ref_t{ value, std::prev(std::end(list)) });
        return *this;
    }

    iterator begin()
    {
        return std::begin(list);
    }

    const_iterator begin() const
    {
        return std::begin(list);
    }

    iterator end()
    {
        return std::end(list);
    }

    const_iterator end() const
    {
        return std::end(list);
    }

    ordered_map_t &erase(const Key &key)
    {
        list.erase(data[key].second);
        data.erase(key);
        return *this;
    }

    ordered_map_t &erase(const iterator &it)
    {
        list.erase(it);
        data.erase(it->first);
        return *this;
    }

    ordered_map_t &erase(const const_iterator &it)
    {
        list.erase(it);
        data.erase(it->first);
        return *this;
    }

    iterator at(std::size_t position)
    {
        iterator it = list.begin();
        for (std::size_t i = 0; (i < position) && (it != list.end()); ++i, ++it) {
        }
        return it;
    }

    const_iterator at(std::size_t position) const
    {
        const_iterator it = list.begin();
        for (std::size_t i = 0; (i < position) && (it != list.end()); ++i, ++it) {
        }
        return it;
    }

    iterator find(const Key &key)
    {
        typename std::map<Key, table_ref_t>::iterator it = data.find(key);
        if (it != data.end())
            return it->second.second;
        return list.end();
    }

    const_iterator find(const Key &key) const
    {
        typename std::map<Key, table_ref_t>::const_iterator it = data.find(key);
        if (it != data.end())
            return it->second.second;
        return list.end();
    }

private:
    std::list<table_node_t> list;
    std::map<Key, table_ref_t> data;
};

} // namespace ordered_map