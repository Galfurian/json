# JSON Parser Library

[![Ubuntu](https://github.com/Galfurian/json/actions/workflows/ubuntu.yml/badge.svg)](https://github.com/Galfurian/json/actions/workflows/ubuntu.yml)
[![Windows](https://github.com/Galfurian/json/actions/workflows/windows.yml/badge.svg)](https://github.com/Galfurian/json/actions/workflows/windows.yml)
[![MacOS](https://github.com/Galfurian/json/actions/workflows/macos.yml/badge.svg)](https://github.com/Galfurian/json/actions/workflows/macos.yml)
[![Sanitizers](https://github.com/Galfurian/json/actions/workflows/sanitizers.yml/badge.svg)](https://github.com/Galfurian/json/actions/workflows/sanitizers.yml)
[![Documentation](https://github.com/Galfurian/json/actions/workflows/documentation.yml/badge.svg)](https://github.com/Galfurian/json/actions/workflows/documentation.yml)

## Overview

This C++ JSON parser library provides functionality to parse and manipulate JSON data. It allows you to read JSON from strings or files, modify the data, and serialize it back to strings or files. The library also provides comprehensive support for various C++ types including custom types, standard containers, and complex types.

The library supports parsing and serializing JSON objects, arrays, strings, numbers, booleans, and null values.

## Features

- Parse JSON data from a string or a file.
- Extract data into C++ types such as `std::string`, `int`, `bool`, `std::complex`, and more.
- Serialize C++ types back into JSON format.
- Supports custom types via operator overloads for `<<` and `>>`.
- Handles both arrays and objects, and supports serialization of complex types such as `std::tuple`, `std::vector`, and `std::map`.
- Accepts a JSON5-style superset of JSON, see the "Accepted syntax" section below.
- Error handling with detailed exception types such as `parser_error`, `type_error`, and `range_error`.

## Installation

The library needs a C++11 compiler and CMake 3.14 or newer. It is made of one header and one source file, so it must be compiled and linked, it is not header-only.

### With CMake (recommended)

```cmake
include(FetchContent)
FetchContent_Declare(json
    GIT_REPOSITORY https://github.com/Galfurian/json.git
    GIT_TAG v3.0.0
)
FetchContent_MakeAvailable(json)

target_link_libraries(my_target PRIVATE json::json)
```

When the library is consumed this way, its tests, examples and documentation are not built, and its warning flags are not applied to your code.

`json::json` also brings the include path of `ordered_map`, a header-only dependency fetched at a pinned commit.

### As an installed package

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build build
cmake --install build
```

```cmake
find_package(json 3 REQUIRED)
target_link_libraries(my_target PRIVATE json::json)
```

The package also installs the `ordered_map` headers that `json.hpp` includes.

### Without CMake

Clone the repository, add `include/` and the `include/` directory of [ordered_map](https://github.com/Galfurian/ordered_map) to your include path, and compile `src/json.cpp` together with your sources.

```bash
git clone https://github.com/Galfurian/json.git
```

Then include the header:

```c++
#include <json/json.hpp>
```

## Usage Examples

### Parsing JSON from a string

```c++
#include <json/json.hpp>
#include <iostream>

int main() {
    const char* json_string = R"(
    {
        // Single-line comment
        "name": "John",

        /* Multi-line comment */
        "age": 30,

        "city": "New York" // Inline comment
    })";

    json::jnode_t root = json::parser::parse(json_string);

    std::string name = root["name"].as_string();
    int age = root["age"].as_number<int>();
    std::string city = root["city"].as_string();

    std::cout << "Name: " << name << "\n";
    std::cout << "Age: " << age << "\n";
    std::cout << "City: " << city << "\n";
    return 0;
}
```

### Parsing JSON from a file

```c++
#include <json/json.hpp>
#include <iostream>

int main() {
    json::jnode_t root = json::parser::parse_file("data.json");

    std::string name = root["name"].as_string();
    int age = root["age"].as_number<int>();
    std::string city = root["city"].as_string();

    std::cout << "Name: " << name << "\n";
    std::cout << "Age: " << age << "\n";
    std::cout << "City: " << city << "\n";
    return 0;
}
```

### Serializing C++ types to JSON

You can serialize standard C++ types and custom types to JSON using the `<<` operator.

```c++
#include <json/json.hpp>
#include <iostream>

struct Person {
    std::string name;
    int age;

    Person(std::string name, int age) : name(name), age(age) {}

    friend json::jnode_t& operator<<(json::jnode_t& lhs, const Person& rhs) {
        lhs.set_type(json::JTYPE_OBJECT);
        lhs["name"] << rhs.name;
        lhs["age"] << rhs.age;
        return lhs;
    }
};

int main() {
    Person p("Alice", 30);
    json::jnode_t node;
    node << p;

    std::cout << node.to_string() << std::endl;
    return 0;
}
```

### File Handling Example (Parsing and Writing JSON Files)

This example demonstrates how to read a JSON file, parse its contents, and then
write the modified JSON back to a file.

```c++
#include <json/json.hpp>
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <path-to-json-file>\n";
        return 1;
    }

    std::string filename(argv[1]);

    // Directly parsing the file using the parse function
    json::jnode_t root = json::parser::parse_file(filename);

    // Modify or inspect the JSON data
    std::cout << "The JSON data read from the file:\n";
    std::cout << root.to_string(true, 2) << "\n";

    // Writing the modified JSON data to a new file
    json::parser::write_file("out-" + filename, root, false);
    std::cout << "Modified JSON written to out-" << filename << "\n";

    return 0;
}
```

While the library allows more granular control through tokenization and manual
parsing (as shown in the next code), most users will find it more
straightforward to use the parse function directly for simplicity and
efficiency.

```c++
#include <json/json.hpp>
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <path-to-json-file>\n";
        return 1;
    }

    std::string filename(argv[1]), content;
    // Read the file.
    if (!json::parser::read_file(filename, content)) {
        std::cerr << "Failed to open file `" << filename << "`\n";
        return 1;
    }

    // Extract tokens.
    std::vector<json::detail::token_t> tokens;
    json::detail::tokenize(content, tokens);

    // Parse tokens
    json::jnode_t root;
    std::size_t k = 0;
    json::detail::json_parse(tokens, 0UL, k, root);

    // Write the file
    json::parser::write_file("out-" + filename, root, false);

    return 0;
}
```

## Supported Types

This library supports the following types for serialization and deserialization:

- `std::string`
- `bool`
- `int`, `unsigned`, `long`, `long long`
- `float`, `double`
- `std::complex`
- `std::tuple`, `std::vector`, `std::list`, `std::set`, `std::map`, `std::unordered_map`
- Custom types (via operator overloads)

## Performance

`benchmarks/bench_parse.cpp` parses and prints a generated 8.7 MB document (50000 records), best of 5 runs, GCC 13 `-O2` (Release):

| Library          | Parse      | Print       |
|------------------|------------|-------------|
| this library     | ~24 MB/s   | ~127 MB/s   |
| nlohmann/json 3.11.3 | ~60 MB/s | ~314 MB/s |

The parser first builds a token list and then the tree, so it needs memory proportional to the number of tokens (about 48 bytes each) on top of the tree. Keep that in mind for documents of hundreds of megabytes, and use `max_input_size`.

Run it with `-DBUILD_BENCHMARKS=ON` (add `-DBENCHMARK_NLOHMANN=ON` for the comparison).

## Accepted syntax

The parser accepts standard JSON plus these JSON5 conveniences:

- `//` and `/* ... */` comments
- single-quoted strings
- unquoted object keys (identifiers made of letters, digits, `_` and `$`)
- trailing commas in objects and arrays
- numbers with a leading `+`, a leading or trailing dot (`.5`, `5.`), in hexadecimal (`0xFF`), and the special values `Infinity`, `-Infinity` and `NaN`
- line continuations: a backslash before a line break inside a string removes the line break

Everything else that is not valid JSON is rejected with a `parser_error`: invalid numbers (`01`, `-`, `1.2.3`, `0x`), bare words used as values, missing commas, unterminated strings and comments, invalid escape sequences, and any data after the root value.

Strings may contain `\uXXXX` escapes, including surrogate pairs. They are decoded to UTF-8.

## Limits and configuration

Global options live in the `json::config` namespace:

| Option                      | Default | Meaning                                                                              |
|-----------------------------|---------|--------------------------------------------------------------------------------------|
| `max_depth`                 | `256`   | Maximum nesting of objects and arrays. Deeper documents raise a `parser_error`.      |
| `max_input_size`            | `0`     | Maximum size in bytes of a document. `0` means no limit.                             |
| `strict_type_check`         | `false` | Throw `type_error` instead of returning a default when a value has a different type. Also reports numbers that do not fit the requested C++ type. |
| `strict_existance_check`    | `false` | Throw instead of returning a null node when a key does not exist.                    |
| `replace_escape_characters` | `false` | Escape special characters when serializing strings.                                  |
| `string_delimiter_character`| `'`     | Quote used when serializing strings.                                                 |

`max_depth` and `max_input_size` can also be passed for a single call, without touching global state:

```c++
json::parse_options options;
options.max_depth      = 32;
options.max_input_size = 1 << 20; // 1 MiB
json::jnode_t root = json::parser::parse(text, options);
json::jnode_t file = json::parser::parse_file("data.json", options);
```

`parse_file` throws `parser_error` if the file cannot be opened.

The other options are global variables: set them once at startup, before using the library from several threads. Parsing and reading a tree that is not modified are otherwise safe to do from different threads on different objects.

When you parse untrusted input, keep `max_depth` low and limit the size of the input you hand to the parser.

## Error Handling

The library uses exceptions to handle errors:

- `parser_error`: General parsing error. It carries the `line` where the problem was found.
- `type_error`: Type mismatch error, derived from `parser_error`.
- `range_error`: Accessing an out-of-bound element, derived from `parser_error`.

On malformed input the parser either returns a tree or throws `parser_error`. This is checked by the test suite, which includes a seeded mutation fuzz test, and by a CI job that runs all the tests under AddressSanitizer and UndefinedBehaviorSanitizer.

## License

This project is licensed under the MIT License - see the `LICENSE.md` file for details.

## Contributions

Contributions are welcome! Please feel free to open issues and pull requests.
