# Changelog

## Unreleased

### Security

- Fix out-of-bounds reads in the parser on truncated input (empty input, an
  unclosed comment, `{`, `[1,` ...).
- Fix stack overflow on deeply nested input. Nesting is now limited by
  `json::config::max_depth` (default 256).

### Added

- Install rules and a CMake package: `find_package(json)` and `json::json`.
  The version is read from `json.hpp`.

- JSON5 numbers: leading `+`, `.5`, `5.`, hexadecimal, `Infinity`, `-Infinity`
  and `NaN`. Numbers are stored in a canonical decimal form.
- `as_number<T>()` reads `1e3` and `2.0` correctly for integer types. It used
  to read `1` and `2`. Integer types get the closest value, or a
  `parser_error` in strict mode, when the number has a fractional part or is
  out of range.

- `json::parse_options` and `parse` / `parse_file` overloads that take it, to
  set `max_depth` and the new `max_input_size` for a single call.
- `json::config::max_input_size` (default 0, no limit).

### Changed

- A line continuation inside a string (backslash and line break) removes the
  line break. It used to be replaced by `\n`.
- `json::parser::parse_file` throws `json::parser_error` when the file cannot
  be opened. It used to return a null node.
- `json::detail::json_parse` takes the maximum depth as an argument.

The parser is stricter. The following inputs used to be accepted, often with
a wrong result, and now raise `json::parser_error`:

- invalid numbers: `-`, `1e`, `1.2.3`, `01`, `0x`
- misspelled literals and bare words used as values: `nul`, `{a: hello}`
- missing separators: `[1 2]`, `{"a":1 "b":2}`
- non-string object keys: `{1:2}`
- unterminated strings and comments
- invalid or unpaired `\u` escapes
- data after the root value

Comments, single-quoted strings, unquoted keys and trailing commas are still
accepted.

`json::jnode_t::as_number<T>()` throws `json::parser_error` when the value
does not fit `T`, but only if `json::config::strict_type_check` is set.

### Fixed

- `\uXXXX` escapes were truncated to one byte. They are now decoded to UTF-8,
  surrogate pairs included.
- An escaped backslash followed by a line break was taken for a line
  continuation.

### Performance

- Tokenizing is about 40% faster (fewer comparisons per character, no
  allocation for plain numbers, one reservation for the token list), and
  strings without escape sequences are not copied twice.
- New benchmark, `-DBUILD_BENCHMARKS=ON`.

- Parsing strings no longer builds a `std::regex` for every value. Parsing
  200000 short strings went from 3.2s to 0.11s.

### Build

- Dependencies are pinned to a commit or tag.
- Warning flags are no longer propagated to projects that link `json::json`.
- Tests, examples and documentation are built by default only when this is
  the top-level CMake project.
- The minimum CMake version is 3.14.

### Testing

- New tests for malformed input, string escapes and a seeded mutation fuzz
  test.
- New CI job running the tests under AddressSanitizer and
  UndefinedBehaviorSanitizer.
