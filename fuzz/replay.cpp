/// @file replay.cpp
/// @author Enrico Fraccaroli (enry.frak@gmail.com)
/// @brief Runs the fuzz target on files, without libFuzzer.
/// @details Used to replay the corpus, and any crash found by the fuzzer, with
/// any compiler.
///
/// @copyright (c) 2024 This file is distributed under the MIT License.
/// See LICENSE.md for details.
///

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t *data, std::size_t size);

int main(int argc, char *argv[])
{
    for (int i = 1; i < argc; ++i) {
        std::ifstream file(argv[i], std::ios::in | std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Cannot open `" << argv[i] << "`.\n";
            return 1;
        }
        std::vector<char> content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        LLVMFuzzerTestOneInput(reinterpret_cast<const std::uint8_t *>(content.data()), content.size());
    }
    return 0;
}
