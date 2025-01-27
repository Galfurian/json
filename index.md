---
layout: default
---

# JSON Parser Library

[![Ubuntu](https://github.com/Galfurian/json/actions/workflows/ubuntu.yml/badge.svg)](https://github.com/Galfurian/json/actions/workflows/ubuntu.yml)
[![Windows](https://github.com/Galfurian/json/actions/workflows/windows.yml/badge.svg)](https://github.com/Galfurian/json/actions/workflows/windows.yml)
[![MacOS](https://github.com/Galfurian/json/actions/workflows/macos.yml/badge.svg)](https://github.com/Galfurian/json/actions/workflows/macos.yml)
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
- Error handling with detailed exception types such as `parser_error`, `type_error`, and `range_error`.

## Installation

Clone the repository and include the necessary header files in your C++ project.

```bash
git clone https://github.com/yourusername/json-parser.git
```

You can then include the `json.hpp` header in your C++ project:

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
        "name": "John",
        "age": 30,
        "city": "New York"
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

## Error Handling

The library uses exceptions to handle errors:

- `parser_error`: General parsing error.
- `type_error`: Type mismatch error.
- `range_error`: Accessing an out-of-bound element.

## License

This project is licensed under the MIT License - see the `LICENSE.md` file for details.

## Contributions

Contributions are welcome! Please feel free to open issues and pull requests.
