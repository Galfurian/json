/// @file stopwatch.hpp
/// @author Enrico Fraccaroli (enry.frak@gmail.com)
/// @brief Benchmarking classes.

#pragma once

#include <iostream>
#include <utility>
#include <iomanip>
#include <chrono>
#include <vector>

namespace stopwatch
{

/// @brief The way the stopwatch prints the elapsed time.
enum PrintMode {
    human,   ///< Human readable :  1h  4m  2s   1m 153u 399n
    numeric, ///< Numeric        :  1.4.2.1.153.399
    total    ///< Total elapsed  :
};

class Duration {
public:
    typedef std::chrono::duration<double> duration_type_t;

    Duration(duration_type_t duration, PrintMode print_mode)
        : _duration(duration),
          _print_mode(print_mode)
    {
        // Nothing to do.
    }

    static inline duration_type_t zero()
    {
        return duration_type_t::zero();
    }

    inline void set_print_mode(PrintMode print_mode)
    {
        _print_mode = print_mode;
    }

    inline Duration operator+(const Duration &rhs) const
    {
        Duration new_duration = *this;
        new_duration._duration += rhs._duration;
        return new_duration;
    }

    template <typename T>
    inline Duration operator+(const T &rhs) const
    {
        Duration new_duration = *this;
        new_duration._duration += rhs;
        return new_duration;
    }

    template <typename T>
    inline Duration operator/(const T &rhs) const
    {
        Duration new_duration = *this;
        new_duration._duration /= rhs;
        return new_duration;
    }

    inline Duration &operator+=(const Duration &rhs)
    {
        _duration += rhs._duration;
        return *this;
    }

    template <typename T>
    inline Duration &operator+=(const T &rhs)
    {
        _duration += rhs;
        return *this;
    }

    template <typename T>
    inline Duration &operator/=(const T &rhs)
    {
        _duration /= rhs;
        return *this;
    }

    inline Duration &operator=(const duration_type_t &rhs)
    {
        _duration = rhs;
        return *this;
    }

    inline std::string to_string() const
    {
        std::stringstream ss;
        // Get the duration.
        auto duration = _duration;
        if (_print_mode == human) {
            auto h = std::chrono::duration_cast<std::chrono::hours>(duration);
            if (h.count()) {
                ss << std::setw(3) << h.count() << "H ";
                duration -= h;
            }
            auto m = std::chrono::duration_cast<std::chrono::minutes>(duration);
            if (m.count()) {
                ss << std::setw(3) << m.count() << "M ";
                duration -= m;
            }
            auto s = std::chrono::duration_cast<std::chrono::seconds>(duration);
            if (s.count()) {
                ss << std::setw(3) << s.count() << "s ";
                duration -= s;
            }
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
            if (ms.count()) {
                ss << std::setw(3) << ms.count() << "m ";
                duration -= ms;
            }
            auto us = std::chrono::duration_cast<std::chrono::microseconds>(duration);
            if (us.count()) {
                ss << std::setw(3) << us.count() << "u ";
                duration -= us;
            }
            auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(duration);
            if (ns.count()) {
                ss << std::setw(3) << ns.count() << "n ";
                duration -= ns;
            }
        } else if (_print_mode == numeric) {
            auto h = std::chrono::duration_cast<std::chrono::hours>(duration);
            ss << h.count() << ".";
            duration -= h;
            auto m = std::chrono::duration_cast<std::chrono::minutes>(duration);
            ss << m.count() << ".";
            duration -= m;
            auto s = std::chrono::duration_cast<std::chrono::seconds>(duration);
            ss << s.count() << ".";
            duration -= s;
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
            ss << ms.count() << ".";
            duration -= ms;
            auto us = std::chrono::duration_cast<std::chrono::microseconds>(duration);
            ss << us.count() << ".";
            duration -= us;
            auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(duration);
            ss << ns.count();
            duration -= ns;
        } else {
            ss << (static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count()) * 1e-09);
        }
        return ss.str();
    }

    friend std::ostream &operator<<(std::ostream &lhs, const Duration &rhs)
    {
        lhs << rhs.to_string();
        return lhs;
    }

private:
    duration_type_t _duration;
    PrintMode _print_mode;
};

class Stopwatch {
public:
    typedef std::chrono::high_resolution_clock clock_type_t;

    Stopwatch(PrintMode print_mode = human)
        : _last_time_point(clock_type_t::now()),
          _total_duration(Duration::zero(), print_mode),
          _partials(),
          _print_mode(print_mode)
    {
        // Nothing to do.
    }

    inline void set_print_mode(PrintMode print_mode)
    {
        _print_mode = print_mode;
        _total_duration.set_print_mode(print_mode);
    }

    inline void reset()
    {
        _last_time_point = clock_type_t::now();
        _total_duration  = std::chrono::duration<double>::zero();
        _partials.clear();
    }

    inline void start()
    {
        _last_time_point = clock_type_t::now();
    }

    inline void round()
    {
        auto now         = clock_type_t::now();
        auto elapsed     = now - _last_time_point;
        _last_time_point = now;
        _total_duration += elapsed;
        _partials.emplace_back(Duration(elapsed, _print_mode));
    }

    Duration last_round() const
    {
        if (_partials.empty())
            return Duration(clock_type_t::now() - _last_time_point, _print_mode);
        return _partials.back();
    }

    Duration total() const
    {
        return _total_duration;
    }

    Stopwatch mean() const
    {
        Stopwatch sw(*this);
        sw._total_duration /= static_cast<double>(_partials.size());
        sw._partials.clear();
        return sw;
    }

    inline std::vector<Duration> partials() const
    {
        return _partials;
    }

    virtual std::string to_string() const
    {
        return _total_duration.to_string();
    }

    inline Duration &operator[](std::size_t position)
    {
        if (position < _partials.size())
            return _partials[position];
        throw std::out_of_range("Out of range of partial times.");
    }

    inline const Duration &operator[](std::size_t position) const
    {
        if (position < _partials.size())
            return _partials[position];
        throw std::out_of_range("Out of range of partial times.");
    }

    friend std::ostream &operator<<(std::ostream &lhs, const Stopwatch &rhs)
    {
        lhs << rhs.to_string();
        return lhs;
    }

private:
    clock_type_t::time_point _last_time_point;
    Duration _total_duration;
    std::vector<Duration> _partials;
    PrintMode _print_mode;
};

/// @brief Runs the function and samples the elapsed time.
/// @param stopwatch the stopwatch used to retrieve the elapse time.
/// @param function the function to sample.
/// @return the same stopwatch passed as argument.
template <class Function>
inline Stopwatch &time(Stopwatch &stopwatch, Function &&function)
{
    stopwatch.reset();
    stopwatch.start();
    function();
    stopwatch.round();
    return stopwatch;
}

/// @brief Runs the function N times and samples the elapsed time.
/// @param stopwatch the stopwatch used to retrieve the elapse time.
/// @param function the function to sample.
/// @return the same stopwatch passed as argument.
template <std::size_t N, class Function>
inline Stopwatch &ntimes(Stopwatch &stopwatch, Function &&function)
{
    stopwatch.reset();
    for (std::size_t i = 0u; i < N; ++i) {
        stopwatch.start();
        function();
        stopwatch.round();
    }
    return stopwatch;
}

} // namespace stopwatch
