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
    typedef std::list<std::pair<Key, Value> > list_t;
    typedef typename list_t::iterator iterator;
    typedef typename list_t::const_iterator const_iterator;
    typedef std::map<Key, iterator> table_t;

    ordered_map_t()
        : list(),
          data()
    {
        // Nothing to do.
    }

    ordered_map_t(const ordered_map_t &other)
        : list(),
          data()
    {
        for (const_iterator it = other.list.begin(); it != other.list.end(); ++it) {
            list.push_back(std::make_pair(it->first, it->second));
            data.insert(std::make_pair(it->first, --(list.end())));
        }
    }

    void clear()
    {
        list.clear();
        data.clear();
    }

    std::size_t size() const
    {
        return list.size();
    }

    iterator set(const Key &key, const Value &value)
    {
        typename table_t::iterator it_table = data.find(key);
        typename list_t::iterator it_list;
        if (it_table == data.end()) {
            list.push_back(std::make_pair(key, value));
            it_list = --(list.end());
            data.insert(std::make_pair(key, it_list));
        } else {
            it_list         = it_table->second;
            it_list->second = value;
        }
        return it_list;
    }

    iterator begin()
    {
        return list.begin();
    }

    const_iterator begin() const
    {
        return list.begin();
    }

    iterator end()
    {
        return list.end();
    }

    const_iterator end() const
    {
        return list.end();
    }

    ordered_map_t &erase(const Key &key)
    {
        typename table_t::iterator it_table = data.find(key);
        if (it_table != data.end()) {
            list.erase(it_table->second);
            data.erase(it_table);
        }
        return *this;
    }

    ordered_map_t &erase(const iterator &it)
    {
        typename table_t::iterator it_table = data.find(it->first);
        if (it_table != data.end()) {
            list.erase(it);
            data.erase(it->first);
        }
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
        typename table_t::iterator it = data.find(key);
        if (it != data.end())
            return it->second;
        return list.end();
    }

    const_iterator find(const Key &key) const
    {
        typename table_t::const_iterator it = data.find(key);
        if (it != data.end())
            return it->second;
        return list.end();
    }

    ordered_map_t &operator=(const ordered_map_t &other)
    {
        this->clear();
        for (const_iterator it = other.list.begin(); it != other.list.end(); ++it) {
            list.push_back(std::make_pair(it->first, it->second));
            data.insert(std::make_pair(it->first, --(list.end())));
        }
        return *this;
    }

private:
    list_t list;
    table_t data;
};

} // namespace ordered_map
