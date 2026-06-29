# Copilot Instructions for md5_implementation

## Project Overview

This repository is a C99 MD5 hash utility with both a command-line interface and an ncurses-based TUI.

- Keep changes aligned with RFC 1321 behavior and lowercase hexadecimal output.
- Preserve cross-platform POSIX compatibility for macOS, Linux, and Windows POSIX environments.
- Favor small, testable modules over monolithic functions.

## Project Layout

- `src/main.c`: CLI entry point and argument parsing.
- `src/md5.c`, `src/md5.h`: Core MD5 implementation.
- `src/file_handler.c`, `src/file_handler.h`: File and stdin handling.
- `src/tui.c`, `src/tui.h`: ncurses-based TUI.
- `src/utils.c`, `src/utils.h`: Shared helpers.
- `src/config.h`: Shared constants and compile-time settings.
- `tests/`: Unit tests, RFC test vectors, and fixtures.
- `Makefile`: Canonical build, test, coverage, and debug workflow.
- `docs/tech_spec.md` and `README.md`: Behavioral and UX expectations.

## C Development Rules

- Target C99 unless an existing file explicitly requires something else.
- Use `-Wall -Wextra -pedantic` cleanly; do not introduce new warnings.
- Prefer `stdint.h`, `stddef.h`, and explicit-width integer types for hashing code.
- Avoid VLAs, hidden global state, and implicit function declarations.
- Check all allocations, file I/O, and ncurses calls that can fail.
- Keep ownership and lifetime obvious. Free resources on every error path.
- Prefer clear helper functions instead of deeply nested control flow.
- Do not change public function names or CLI flags unless the task requires it.

## MD5-Specific Guidance

- Preserve RFC 1321 semantics exactly for padding, round constants, bit counts, and little-endian word handling.
- Validate changes against known MD5 test vectors, including empty input, short strings, long inputs, and binary data.
- Be careful with signedness, integer overflow assumptions, and byte-order conversions.
- Keep digest formatting in canonical 32-character lowercase hexadecimal form.

## TUI Guidance

- Use `ncurses` abstractions instead of raw terminal escape sequences.
- Keep rendering, input handling, and screen state transitions separated where practical.
- TUI code must handle small terminal sizes gracefully and avoid drawing outside window bounds.
- Do not block the UI unnecessarily during file operations; provide clear status or error feedback.
- Preserve keyboard navigation described in the spec: arrows, Enter, numeric shortcuts, and exit controls.
- When logic can be separated from rendering, move it into testable helpers outside direct ncurses calls.

## Make and Build Workflow

- Treat the top-level `Makefile` as the source of truth for build and test commands.
- Prefer updating existing targets and variables instead of adding ad hoc shell scripts.
- Reuse existing variables such as `CC`, `CFLAGS`, `LDFLAGS`, `SRC_DIR`, `OBJ_DIR`, and `BIN_DIR`.
- Keep GCC and Clang compatibility when changing compiler flags.
- If a change adds a new source file, update the relevant `SOURCES`, `HEADERS`, or test source lists.
- If a change adds a new workflow, expose it as a Make target and document it in `README.md` when user-facing.

Use these commands when validating work:

```bash
make
make debug
make test
make test-md5
make test-file-handler
make test-tui
make test-vectors
make coverage
make test-valgrind
```

## Testing Expectations

- Add or update tests for every behavior change in hashing, file handling, parsing, or TUI state logic.
- Prefer narrow unit tests in `tests/` before broader end-to-end coverage.
- For MD5 changes, include RFC 1321 vectors and edge cases around padding boundaries.
- For file handling changes, test text files, binary files, stdin, missing files, and large inputs.
- For TUI changes, favor testing state transitions and helper logic rather than fragile screen-paint snapshots.
- Run the narrowest relevant target first, then `make test` for final validation.
- If memory safety is at risk, prefer `make debug` and `make test-valgrind` when available.

## Documentation Expectations

- Update `README.md` when changing CLI flags, build steps, Make targets, or end-user behavior.
- Update `docs/tech_spec.md` when changing architecture, module responsibilities, or TUI flows.
- Keep examples in sync with actual command names and supported options.

## Change Discipline

- Make the smallest change that solves the root problem.
- Preserve existing code style within each file.
- Avoid unrelated refactors while fixing a focused issue.
- If the repo documentation and implementation conflict, call out the mismatch and align both when appropriate.