# Changelog

## Unreleased

### Security

- Fix out-of-bounds reads in the parser on truncated input (empty input, an
  unclosed comment, `{`, `[1,` ...).
- Fix stack overflow on deeply nested input. Nesting is now limited by
  `json::config::max_depth` (default 256).

### Added

- `json::parse_options` and `parse` / `parse_file` overloads that take it, to
  set `max_depth` and the new `max_input_size` for a single call.
- `json::config::max_input_size` (default 0, no limit).

### Changed

- `json::parser::parse_file` throws `json::parser_error` when the file cannot
  be opened. It used to return a null node.
- `json::detail::json_parse` takes the maximum depth as an argument.

The parser is stricter. The following inputs used to be accepted, often with
a wrong result, and now raise `json::parser_error`:

- invalid numbers: `-`, `1e`, `1.`, `1.2.3`, `01`
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
