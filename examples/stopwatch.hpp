/// @file stopwatch.hpp
/// @author Enrico Fraccaroli (enry.frak@gmail.com)
/// @brief Benchmarking classes.

#pragma once

#include <iostream>
#include <utility>
#include <iomanip>
#include <vector>
#include <sstream>

#if __cplusplus < 201103L
#include <time.h>
#else
#include <chrono>
#endif

namespace stopwatch
{

/// @brief The way the stopwatch prints the elapsed time.
enum PrintMode {
    human,   ///< Human readable :  1h  4m  2s   1m 153u 399n
    numeric, ///< Numeric        :  1.4.2.1.153.399
    total    ///< Total elapsed  :
};

#if __cplusplus < 201103L

namespace detail
{

static const unsigned long ns_per_sec = 1000000000UL;

inline timespec normalize(timespec ts)
{
    while (ts.tv_nsec >= static_cast<long>(ns_per_sec)) {
        ++(ts.tv_sec);
        ts.tv_nsec -= ns_per_sec;
    }
    while (ts.tv_nsec <= -static_cast<long>(ns_per_sec)) {
        --(ts.tv_sec);
        ts.tv_nsec += ns_per_sec;
    }
    if (ts.tv_nsec < 0) {
        --(ts.tv_sec);
        ts.tv_nsec = (ns_per_sec + ts.tv_nsec);
    }
    return ts;
}

template <typename T>
inline timespec ns_to_nanoseconds(const T &ns)
{
    timespec ts;
    ts.tv_sec  = static_cast<time_t>(ns) / detail::ns_per_sec;
    ts.tv_nsec = static_cast<long>(ns) % detail::ns_per_sec;
    return detail::normalize(ts);
}

template <typename T>
inline T ts_to_nanoseconds(const timespec &ts)
{
    return static_cast<T>((ts.tv_sec * detail::ns_per_sec) + ts.tv_nsec);
}
template <typename T>
inline T ts_to_microseconds(const timespec &ts)
{
    unsigned long ns = ts_to_nanoseconds<unsigned long>(ts);
    return static_cast<T>(ns / 1000UL);
}
template <typename T>
inline T ts_to_milliseconds(const timespec &ts)
{
    unsigned long ns = ts_to_nanoseconds<unsigned long>(ts);
    return static_cast<T>(ns / 1000000UL);
}
template <typename T>
inline T ts_to_seconds(const timespec &ts)
{
    unsigned long ns = ts_to_nanoseconds<unsigned long>(ts);
    return static_cast<T>(ns / detail::ns_per_sec);
}
template <typename T>
inline T ts_to_minutes(const timespec &ts)
{
    unsigned long ns = ts_to_nanoseconds<unsigned long>(ts);
    return static_cast<T>((ns / detail::ns_per_sec) / 60);
}
template <typename T>
inline T ts_to_hours(const timespec &ts)
{
    unsigned long ns = ts_to_nanoseconds<unsigned long>(ts);
    return static_cast<T>((ns / detail::ns_per_sec) / 3600);
}

inline void clock_gettime(timespec &ts)
{
    clock_gettime(CLOCK_REALTIME, &ts);
}

inline timespec clock_now()
{
    timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts;
}

} // namespace detail

inline timespec operator+(const timespec &lhs, const timespec &rhs)
{
    return detail::ns_to_nanoseconds(
        detail::ts_to_nanoseconds<unsigned long>(lhs) +
        detail::ts_to_nanoseconds<unsigned long>(rhs));
}

template <typename T>
inline timespec operator+(const timespec &lhs, const T &rhs)
{
    return detail::ns_to_nanoseconds(detail::ts_to_nanoseconds<T>(lhs) + rhs);
}

inline timespec operator-(const timespec &lhs, const timespec &rhs)
{
    return detail::ns_to_nanoseconds(
        detail::ts_to_nanoseconds<unsigned long>(lhs) -
        detail::ts_to_nanoseconds<unsigned long>(rhs));
}

template <typename T>
inline timespec operator-(const timespec &lhs, const T &rhs)
{
    return detail::ns_to_nanoseconds(detail::ts_to_nanoseconds<T>(lhs) - rhs);
}

inline timespec operator/(const timespec &lhs, const timespec &rhs)
{
    return detail::ns_to_nanoseconds(
        detail::ts_to_nanoseconds<unsigned long>(lhs) /
        detail::ts_to_nanoseconds<unsigned long>(rhs));
}

template <typename T>
inline timespec operator/(const timespec &lhs, const T &rhs)
{
    return detail::ns_to_nanoseconds(detail::ts_to_nanoseconds<T>(lhs) / rhs);
}

inline timespec &operator+=(timespec &lhs, const timespec &rhs)
{
    return (lhs = lhs + rhs);
}

template <typename T>
inline timespec &operator+=(timespec &lhs, const T &rhs)
{
    return (lhs = lhs + rhs);
}

inline timespec &operator-=(timespec &lhs, const timespec &rhs)
{
    return (lhs = lhs - rhs);
}

template <typename T>
inline timespec &operator-=(timespec &lhs, const T &rhs)
{
    return (lhs = lhs - rhs);
}

inline timespec &operator/=(timespec &lhs, const timespec &rhs)
{
    return (lhs = lhs / rhs);
}

template <typename T>
inline timespec &operator/=(timespec &lhs, const T &rhs)
{
    return (lhs = lhs / rhs);
}

#endif

class Duration {
public:
#if __cplusplus < 201103L
    typedef timespec duration_type_t;
#else
    typedef std::chrono::duration<double> duration_type_t;
#endif

    Duration(duration_type_t duration, PrintMode print_mode)
        : _duration(duration),
          _print_mode(print_mode)
    {
        // Nothing to do.
    }

    static inline duration_type_t zero()
    {
#if __cplusplus < 201103L
        duration_type_t duration;
        duration.tv_sec  = 0;
        duration.tv_nsec = 0;
        return duration;
#else
        return duration_type_t::zero();
#endif
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
        duration_type_t duration = _duration;
        if (_print_mode == human) {
#if __cplusplus < 201103L
            unsigned h = detail::ts_to_hours<unsigned>(duration);
            if (h) {
                ss << std::setw(3) << h << "H ";
                duration -= (h * 3600) * detail::ns_per_sec;
            }
            unsigned m = detail::ts_to_minutes<unsigned>(duration);
            if (m) {
                ss << std::setw(3) << m << "M ";
                duration -= (m * 60) * detail::ns_per_sec;
            }
            unsigned s = detail::ts_to_seconds<unsigned>(duration);
            if (s) {
                ss << std::setw(3) << s << "s ";
                duration -= s * detail::ns_per_sec;
            }
            unsigned ms = detail::ts_to_milliseconds<unsigned>(duration);
            if (ms) {
                ss << std::setw(3) << ms << "m ";
                duration -= ms * 1000000UL;
            }
            unsigned us = detail::ts_to_microseconds<unsigned>(duration);
            if (us) {
                ss << std::setw(3) << us << "u ";
                duration -= us * 1000UL;
            }
            unsigned ns = detail::ts_to_nanoseconds<unsigned>(duration);
            if (ns) {
                ss << std::setw(3) << ns << "n ";
                duration -= ns;
            }
#else
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
#endif
        } else if (_print_mode == numeric) {
#if __cplusplus < 201103L
            unsigned h = detail::ts_to_hours<unsigned>(duration);
            ss << h << ".";
            duration -= (h * 3600) * detail::ns_per_sec;
            unsigned m = detail::ts_to_minutes<unsigned>(duration);
            ss << m << ".";
            duration -= (m * 60) * detail::ns_per_sec;
            unsigned s = detail::ts_to_seconds<unsigned>(duration);
            ss << s << ".";
            duration -= s * detail::ns_per_sec;
            unsigned ms = detail::ts_to_milliseconds<unsigned>(duration);
            ss << ms << ".";
            duration -= ms * 1000000UL;
            unsigned us = detail::ts_to_microseconds<unsigned>(duration);
            ss << us << ".";
            duration -= us * 1000UL;
            unsigned ns = detail::ts_to_nanoseconds<unsigned>(duration);
            ss << ns << ".";
            duration -= ns;
#else
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
#endif
            duration -= ns;
        } else {
#if __cplusplus < 201103L
            ss << detail::ts_to_nanoseconds<double>(duration) * 1e-09;
#else
            ss << (static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count()) * 1e-09);
#endif
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
#if __cplusplus < 201103L
    typedef timespec clock_type_t;
    typedef timespec time_point_type_t;
#else
    typedef std::chrono::high_resolution_clock clock_type_t;
    typedef clock_type_t::time_point time_point_type_t;
#endif

    Stopwatch(PrintMode print_mode = human)
        : _last_time_point(),
          _total_duration(Duration::zero(), print_mode),
          _partials(),
          _print_mode(print_mode)
    {
#if __cplusplus < 201103L
        detail::clock_gettime(_last_time_point);
#else
        _last_time_point                 = clock_type_t::now();
#endif
    }

    inline void set_print_mode(PrintMode print_mode)
    {
        _print_mode = print_mode;
        _total_duration.set_print_mode(print_mode);
    }

    inline void reset()
    {
#if __cplusplus < 201103L
        _total_duration = Duration(Duration::zero(), _print_mode);
#else
        _total_duration                  = std::chrono::duration<double>::zero();
#endif
        _partials.clear();
        // Star again the timer.
        this->start();
    }

    inline void start()
    {
#if __cplusplus < 201103L
        detail::clock_gettime(_last_time_point);
#else
        _last_time_point                 = clock_type_t::now();
#endif
    }

    inline void round()
    {
#if __cplusplus < 201103L
        time_point_type_t now     = detail::clock_now();
        time_point_type_t elapsed = now - _last_time_point;
#else
        time_point_type_t now            = clock_type_t::now();
        std::chrono::nanoseconds elapsed = now - _last_time_point;
#endif
        _last_time_point = now;
        _total_duration += elapsed;
        _partials.push_back(Duration(elapsed, _print_mode));
    }

    Duration last_round() const
    {
        if (_partials.empty()) {
#if __cplusplus < 201103L
            time_point_type_t now = detail::clock_now();
#else
            time_point_type_t now = clock_type_t::now();
#endif
            return Duration(now - _last_time_point, _print_mode);
        }
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
    time_point_type_t _last_time_point;
    Duration _total_duration;
    std::vector<Duration> _partials;
    PrintMode _print_mode;
};

/// @brief Runs the function and samples the elapsed time.
/// @param stopwatch the stopwatch used to retrieve the elapse time.
/// @param function the function to sample.
/// @return the same stopwatch passed as argument.
template <class Function>
inline Stopwatch &time(Stopwatch &stopwatch, const Function &function)
{
    stopwatch.reset();
    function();
    stopwatch.round();
    return stopwatch;
}

/// @brief Runs the function N times and samples the elapsed time.
/// @param stopwatch the stopwatch used to retrieve the elapse time.
/// @param function the function to sample.
/// @return the same stopwatch passed as argument.
template <std::size_t N, class Function>
inline Stopwatch &ntimes(Stopwatch &stopwatch, const Function &function)
{
    stopwatch.reset();
    for (std::size_t i = 0u; i < N; ++i) {
        function();
        stopwatch.round();
    }
    return stopwatch;
}

} // namespace stopwatch
