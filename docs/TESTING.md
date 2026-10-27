# Validation record

Validated on 27 October 2026 with GCC 13.3.0 on Linux.

- The maintained code builds with `-Wall -Wextra -Wpedantic -Werror`.
- `make test` passed in this environment.
- Tests run locally; they do not contact public services.
- Build products are excluded from version control.

The C++ unit checks and two Python CLI tests passed. With seed 10, XOR reached zero mismatches in 90 generations (4 output bits), and the small 101-detector table reached zero mismatches in 172 generations (24 output bits). These are test examples, not reproductions of the thesis benchmark experiments.

The C++ core tests also passed with AddressSanitizer and UndefinedBehaviorSanitizer. Leak detection was disabled because LeakSanitizer is not supported under this runtime's tracing environment; no leak-check result is claimed.
