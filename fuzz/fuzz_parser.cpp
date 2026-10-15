/// @file fuzz_parser.cpp
/// @author Enrico Fraccaroli (enry.frak@gmail.com)
/// @brief libFuzzer entry point for the parser.
/// @details The only acceptable outcomes are a tree or a json::parser_error.
/// The tree is also read and serialized, to exercise the accessors.
///
/// @copyright (c) 2024 This file is distributed under the MIT License.
/// See LICENSE.md for details.
///

#include <cstddef>
#include <cstdint>
#include <json/json.hpp>
#include <string>

/// @brief Reads every value of the tree.
static void visit(const json::jnode_t &node)
{
    switch (node.get_type()) {
    case json::JTYPE_ARRAY:
    case json::JTYPE_OBJECT:
        for (std::size_t i = 0; i < node.size(); ++i) {
            visit(node[i]);
        }
        break;
    case json::JTYPE_STRING:
        (void)node.as_string();
        break;
    case json::JTYPE_NUMBER:
        (void)node.as_number<int>();
        (void)node.as_number<unsigned long long>();
        (void)node.as_number<double>();
        break;
    default:
        break;
    }
}

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t *data, std::size_t size)
{
    // Keep the inputs small, the parser is not the bottleneck we are looking for.
    if (size > (1U << 16U)) {
        return 0;
    }
    std::string input(reinterpret_cast<const char *>(data), size);
    try {
        json::parse_options options;
        options.max_depth  = 64;
        json::jnode_t root = json::parser::parse(input, options);
        visit(root);
        // What we print must be parsed back.
        std::string printed = root.to_string(false);
        (void)json::parser::parse(printed, options);
    } catch (const json::parser_error &) {
        // Rejecting the input is fine.
    }
    return 0;
}
