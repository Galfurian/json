# Security Policy

## Supported versions

Security fixes are made on the latest release.

## Reporting a vulnerability

Please do not open a public issue for a security problem. Use the
[private vulnerability reporting](https://github.com/Galfurian/json/security/advisories/new)
form of this repository, and include the input that triggers the problem and
the compiler you used.

## Parsing untrusted input

- The parser returns a tree or throws `json::parser_error`. Catch it.
- Nesting is limited by `json::config::max_depth` (default 256). Lower it if
  you do not need deep documents.
- The parser keeps the whole document in memory, in addition to the tree
  built from it. Limit the size of the input you accept.
- `json::config` holds global variables. Set them once at startup.
