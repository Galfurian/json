#include <json/json.hpp>
#include <stopwatch/stopwatch.hpp>

#include <iostream>
#include <sstream>

int main(int argc, char *argv[])
{
    if (argc == 2) {
        std::string input(argv[1]);
        stopwatch::Stopwatch sw;
        sw.set_print_mode(stopwatch::human);
        sw.start();
        json::jnode_t root = json::parser::parse_file(input);
        sw.round();
        json::parser::write_file("out-" + input, root, false);
        sw.round();
        std::cout << "The file parsing took : " << sw[0] << "\n";
        std::cout << "The writing took      : " << sw[1] << "\n";
        std::cout << "In total it took : " << sw.total() << "\n";
    }
    return 0;
}