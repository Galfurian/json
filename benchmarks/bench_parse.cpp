/// @file bench_parse.cpp
/// @author Enrico Fraccaroli (enry.frak@gmail.com)
/// @brief Measures parsing and serialization on a generated document.
/// @details The document is deterministic. If the build defines
/// JSON_BENCH_NLOHMANN, nlohmann/json is measured on the same text.
///
/// @copyright (c) 2024 This file is distributed under the MIT License.
/// See LICENSE.md for details.
///

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <iostream>
#include <json/json.hpp>
#include <string>
#include <vector>

#ifdef JSON_BENCH_NLOHMANN
#include <nlohmann/json.hpp>
#endif

/// @brief Builds an array of records, similar to a typical API response.
static std::string make_document(std::size_t records)
{
    std::string text = "[";
    for (std::size_t i = 0; i < records; ++i) {
        if (i != 0) {
            text += ",";
        }
        text += "{\"id\":" + std::to_string(i) + ",\"name\":\"user-" + std::to_string(i) +
                "\",\"email\":\"user" + std::to_string(i) + "@example.com\",\"score\":" +
                std::to_string(static_cast<double>(i) * 1.5) + ",\"active\":" + ((i % 2) != 0 ? "true" : "false") +
                ",\"tags\":[\"a\",\"b\",\"c\"],\"address\":{\"city\":\"Verona\",\"zip\":\"37129\"},\"note\":null}";
    }
    return text + "]";
}

/// @brief Runs the function a few times and returns the best time in seconds.
template <typename Function>
static double best_of(unsigned runs, Function function)
{
    double best = 1e30;
    for (unsigned i = 0; i < runs; ++i) {
        auto start = std::chrono::steady_clock::now();
        function();
        std::chrono::duration<double> elapsed = std::chrono::steady_clock::now() - start;
        best                                  = std::min(best, elapsed.count());
    }
    return best;
}

static void report(const char *name, double seconds, std::size_t bytes)
{
    std::printf("%-28s %8.1f ms %10.1f MB/s\n", name, seconds * 1e3, static_cast<double>(bytes) / seconds / 1e6);
}

int main(int argc, char *argv[])
{
    std::size_t records = (argc > 1) ? static_cast<std::size_t>(std::stoul(argv[1])) : 50000;
    std::string text    = make_document(records);
    std::printf("document: %zu records, %.1f MB\n", records, static_cast<double>(text.size()) / 1e6);

    // Keep the results alive, so that the compiler cannot drop the work.
    std::size_t sink = 0;

    report("json parse", best_of(5, [&]() { sink += json::parser::parse(text).size(); }), text.size());
    json::jnode_t tree = json::parser::parse(text);
    report("json to_string", best_of(5, [&]() { sink += tree.to_string(false).size(); }), text.size());

#ifdef JSON_BENCH_NLOHMANN
    report("nlohmann parse", best_of(5, [&]() { sink += nlohmann::json::parse(text).size(); }), text.size());
    nlohmann::json other = nlohmann::json::parse(text);
    report("nlohmann dump", best_of(5, [&]() { sink += other.dump().size(); }), text.size());
#endif

    return (sink == 0) ? 1 : 0;
}
