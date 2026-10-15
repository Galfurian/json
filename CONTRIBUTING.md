# Contributing

## Building and testing

```bash
cmake -B build -DWARNINGS_AS_ERRORS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

Run the tests with the sanitizers too, the CI does:

```bash
cmake -B build-san -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-sanitize-recover=undefined" \
    -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address,undefined"
cmake --build build-san && ctest --test-dir build-san --output-on-failure
```

## Formatting

The `.clang-format` file needs clang-format 20. The one shipped by many
distributions is older and rejects the file, install it with pip:

```bash
pip install clang-format==20.1.8
clang-format -i include/json/json.hpp src/json.cpp tests/*.cpp
```

## Static analysis

`.github/clang-tidy-ci.yml` holds the checks that the CI enforces:

```bash
cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
clang-tidy --config-file=.github/clang-tidy-ci.yml -p build src/json.cpp
```

## Changes to the parser

- Add a test for the malformed input you fixed to `tests/test_malformed.cpp`,
  or an input file to `fuzz/corpus`: the corpus is replayed by the tests.
- Parsing malformed input must throw `json::parser_error`, never crash.
- Update `CHANGELOG.md`.

## Workflow

Work on a branch, open a pull request against `develop`. Pull requests are
merged with a merge commit.
