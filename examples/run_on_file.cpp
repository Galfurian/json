/// @file run_on_file.cpp
/// @author Enrico Fraccaroli (enry.frak@gmail.com)
/// @brief Shows an example of how to use the json library on file.
/// 
/// @copyright (c) 2024 This file is distributed under the MIT License.
/// See LICENSE.md for details.
/// 

#include <json/json.hpp>
#include <timelib/stopwatch.hpp>

#include <iostream>
#include <sstream>

int main(int argc, char *argv[])
{
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <path-to-json-file>\n";
        return 1;
    }
    // Prepare the stopwatch.
    timelib::Stopwatch sw;
    sw.set_print_mode(timelib::human);
    sw.start();

    // Prepare the variables we are going to use.
    std::vector<json::detail::token_t> tokens;
    std::string filename(argv[1]), content;
    std::size_t k = 0;

    // Read the file.
    if (!json::parser::read_file(filename, content)) {
        std::cerr << "Failed to open file `" << filename << "`\n";
        return 1;
    }
    std::cout << "The reading took           : " << sw.round() << "\n";

    // Extract the tokens.
    json::detail::tokenize(content, tokens);
    std::cout << "Extracting the tokens took : " << sw.round() << "\n";

    // Parse the tokens.
    json::jnode_t root;
    json::detail::json_parse(tokens, 0UL, k, root);
    std::cout << "Parsing the tokens took    : " << sw.round() << "\n";

    // Write the file.
    json::parser::write_file("out-" + filename, root, false);
    std::cout << "The writing took           : " << sw.round() << "\n";

    std::cout << "In total it took           : " << sw.total() << "\n";

    std::cout << root << "\n";
    std::cout << root["c"] << "\n";
    return 0;
}