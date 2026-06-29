# Build Guide

## Requirements

- C99-compatible compiler
- GNU Make
- ncurses development headers and library

## Common Commands

```bash
make
make debug
make test
make coverage
make clean
```

## Notes

- The top-level `Makefile` is the canonical build entry point.
- `make test-fixtures` generates reusable sample files under `tests/fixtures`.
- `make docs` expects Doxygen and a `Doxyfile` in the repository root.