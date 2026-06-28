# MD5 Hash Implementation - Technical Specification

## 1. Overview

This document provides a detailed technical specification for a command-line C program that implements MD5 hashing functionality with an interactive Text User Interface (TUI). The program shall support computing MD5 hashes for raw text input, text files, and binary files.

**Project Name:** md5_implementation  
**Language:** C (C99 or later)  
**Target Platforms:** Linux, macOS, Windows (with POSIX compliance)  
**License:** MIT License

---

## 2. Functional Requirements

### 2.1 Core MD5 Functionality

The program must implement the MD5 message-digest algorithm as defined in **RFC 1321**.

#### 2.1.1 MD5 Algorithm Implementation
- **Input Support:**
  - Raw text strings (command-line arguments)
  - Text files (UTF-8 and ASCII encoded)
  - Binary files (any format)
  - Standard input (stdin)

- **Output:**
  - 128-bit hash output represented as a 32-character hexadecimal string
  - Canonical lowercase hexadecimal format (e.g., `5d41402abc4b2a76b9719d911017c592`)

- **Algorithm Compliance:**
  - Proper initialization of MD5 state variables (A, B, C, D)
  - All 64 MD5 operation rounds with correct constants and transformations
  - Correct padding for input messages
  - Proper handling of 64-bit length field in little-endian format

### 2.2 Input Modes

#### 2.2.1 Command-Line Mode
```
Usage: md5 [OPTIONS] [INPUT]

OPTIONS:
  -t, --text STRING        Hash the provided text string
  -f, --file PATH          Hash the specified file
  -b, --binary PATH        Hash binary file (outputs hex digest)
  -s, --stdin              Read from standard input
  -q, --quiet              Output only the hash digest
  -v, --verbose            Output detailed information
  -h, --help               Display help message
  --version                Display program version
```

#### 2.2.2 Interactive TUI Mode
- Launched when no arguments are provided
- Menu-driven interface for selecting input mode
- Real-time hash computation with progress indication
- Ability to switch between modes without exiting

### 2.3 Output Formats

#### 2.3.1 Standard Output
```
MD5 Hash Result
===============
Input: [filename or description]
Hash:  5d41402abc4b2a76b9719d911017c592
Size:  [bytes/size information]
```

#### 2.3.2 Quiet Mode Output
```
5d41402abc4b2a76b9719d911017c592
```

#### 2.3.3 Verbose Mode Output
```
MD5 Hash Implementation v1.0
File: /path/to/file
File Size: 1024 bytes
Processing Time: 2.345 ms
Blocks Processed: 1
MD5 Digest: 5d41402abc4b2a76b9719d911017c592
Verification: [status]
```

---

## 3. Text User Interface (TUI) Specification

### 3.1 TUI Architecture

The TUI shall utilize the **ncurses** library for cross-platform terminal control and shall follow a state-machine design pattern.

#### 3.1.1 TUI States
1. **Main Menu** - Primary navigation screen
2. **Hash Text** - Text input screen
3. **Hash File** - File browser/selection screen
4. **Hash Stdin** - Stdin reading screen
5. **Results** - Display hash results
6. **Settings** - Configuration options
7. **About** - Program information

### 3.2 Main Menu Screen

```
┌─────────────────────────────────────────┐
│         MD5 Hash Utility v1.0            │
├─────────────────────────────────────────┤
│                                         │
│  Welcome to the MD5 Hash Utility        │
│                                         │
│  Select an option:                      │
│                                         │
│  [1] Hash Text String                   │
│  [2] Hash File                          │
│  [3] Hash Binary File                   │
│  [4] Read from Standard Input           │
│  [5] Settings                           │
│  [6] About                              │
│  [7] Exit                               │
│                                         │
│  Use arrow keys or numbers to select.   │
│  Press Enter to confirm.                │
│                                         │
└─────────────────────────────────────────┘
```

### 3.3 Text Input Screen

```
┌─────────────────────────────────────────┐
│  Hash Text String                       │
├─────────────────────────────────────────┤
│                                         │
│  Enter text to hash (Ctrl+D to finish): │
│  ┌─────────────────────────────────────┐│
│  │ [User input area - multiline]       ││
│  │                                     ││
│  └─────────────────────────────────────┘│
│                                         │
│  [C]ompute  [C]lear  [B]ack             │
│                                         │
└─────────────────────────────────────────┘
```

### 3.4 File Browser Screen

```
┌──────────────────────────────────────────┐
│  Select File to Hash                     │
├──────────────────────────────────────────┤
│  Current: /home/user/documents/          │
│                                          │
│  [D] ..                                  │
│  [D] Desktop                             │
│  [D] Downloads                           │
│  [F] document.txt        (5.2 KB)        │
│  [F] archive.zip         (12.4 MB)       │
│  [F] image.png           (2.1 MB)        │
│                                          │
│  Use ↑/↓ to navigate, Enter to select    │
│  [/] Search  [R]efresh  [B]ack           │
│                                          │
└──────────────────────────────────────────┘
```

### 3.5 Results Screen

```
┌──────────────────────────────────────────┐
│  Hash Result                             │
├──────────────────────────────────────────┤
│                                          │
│  File: document.txt                      │
│  Size: 5,242 bytes                       │
│  Time: 1.234 ms                          │
│                                          │
│  MD5 Digest:                             │
│  ┌──────────────────────────────────────┐│
│  │ 5d41402abc4b2a76b9719d911017c592 │││
│  └──────────────────────────────────────┘│
│                                          │
│  [C]opy to Clipboard  [S]ave  [B]ack     │
│                                          │
└──────────────────────────────────────────┘
```

### 3.6 TUI Features

- **Navigation:** Arrow keys, Tab, Enter, numeric shortcuts
- **Color Support:** Syntax highlighting for hash output
- **Responsive Design:** Adapts to terminal size (minimum 80x24)
- **Error Handling:** Non-blocking error dialogs with user acknowledgment
- **Progress Indication:** Visual feedback for long-running operations
- **Clipboard Integration:** Copy results to system clipboard (platform-specific)
- **History:** Recent files and searches (last 10 entries)

---

## 4. Architecture and Design

### 4.1 Module Structure

```
md5_implementation/
├── src/
│   ├── main.c               # Entry point, argument parsing
│   ├── md5.c                # Core MD5 algorithm
│   ├── md5.h                # MD5 function declarations
│   ├── file_handler.c       # File I/O operations
│   ├── file_handler.h       # File handler declarations
│   ├── tui.c                # Text User Interface
│   ├── tui.h                # TUI function declarations
│   ├── utils.c              # Utility functions
│   ├── utils.h              # Utility declarations
│   └── config.h             # Configuration constants
├── tests/
│   ├── test_md5.c           # MD5 algorithm tests
│   ├── test_file_handler.c  # File handler tests
│   ├── test_tui.c           # TUI tests
│   ├── test_vectors.c       # RFC 1321 test vectors
│   ├── test_fixtures/       # Test data files
│   └── Makefile             # Test build configuration
├── docs/
│   ├── RFC1321.md           # MD5 RFC specification
│   ├── BUILD.md             # Build instructions
│   └── USAGE.md             # User manual
├── Makefile                 # Build configuration
├── tech_spec.md             # This file
├── README.md                # Project overview
└── LICENSE                  # MIT License
```

### 4.2 MD5 Module (`md5.c`, `md5.h`)

#### Data Structures

```c
typedef struct {
    uint32_t A;              // State variable A
    uint32_t B;              // State variable B
    uint32_t C;              // State variable C
    uint32_t D;              // State variable D
    uint64_t count;          // Number of bits processed
    uint8_t buffer[64];      // Current 512-bit block buffer
} MD5_CTX;
```

#### Core Functions

```c
void md5_init(MD5_CTX *ctx);
void md5_update(MD5_CTX *ctx, const uint8_t *input, size_t length);
void md5_final(MD5_CTX *ctx, uint8_t digest[16]);
void md5_string(const char *str, uint8_t digest[16]);
void md5_file(const char *filename, uint8_t digest[16]);
```

### 4.3 File Handler Module (`file_handler.c`, `file_handler.h`)

#### Functions

```c
typedef enum {
    FILE_MODE_TEXT,
    FILE_MODE_BINARY
} FileMode;

int file_exists(const char *path);
size_t file_size(const char *path);
int hash_file(const char *path, FileMode mode, uint8_t digest[16]);
int read_stdin(uint8_t digest[16]);
```

### 4.4 TUI Module (`tui.c`, `tui.h`)

#### Functions

```c
typedef enum {
    STATE_MAIN_MENU,
    STATE_HASH_TEXT,
    STATE_HASH_FILE,
    STATE_HASH_STDIN,
    STATE_RESULTS,
    STATE_SETTINGS,
    STATE_ABOUT
} UIState;

int tui_init(void);
void tui_cleanup(void);
void tui_run(void);
void tui_display_menu(void);
void tui_display_text_input(void);
void tui_display_file_browser(void);
void tui_display_results(const char *input, const uint8_t digest[16]);
void tui_handle_input(int ch);
```

### 4.5 Utility Module (`utils.c`, `utils.h`)

#### Functions

```c
void hex_encode(const uint8_t *data, size_t len, char *output);
void hex_decode(const char *hex, uint8_t *output, size_t *len);
char* format_file_size(size_t bytes);
double get_time_ms(void);
int clipboard_copy(const char *text);
int validate_input(const char *input, size_t max_len);
```

---

## 5. Implementation Details

### 5.1 MD5 Algorithm Constants

The implementation must include the following MD5 constants:

- **T[64]:** Sine-based constants for each operation round
- **S[64]:** Shift amounts for left rotate operations
- **Zr[64]:** Block selection indices for each round

### 5.2 Platform-Specific Considerations

#### 5.2.1 Endianness Handling
- Detect and handle both little-endian and big-endian systems
- Proper byte conversion for 64-bit length field

#### 5.2.2 File I/O
- Support paths with Unicode characters
- Handle files larger than available RAM using buffered reading
- Proper error handling for permission denied, file not found, etc.

#### 5.2.3 Terminal/TUI
- Detect terminal capabilities (color support, size)
- Graceful degradation for limited terminal features
- Handle terminal resize events (SIGWINCH)

---

## 6. Testing Specification

### 6.1 Test Categories

#### 6.1.1 Unit Tests - MD5 Algorithm

**RFC 1321 Test Vectors:**

```
Input:  ""
Output: d41d8cd98f00b204e9800998ecf8427e

Input:  "a"
Output: 0cc175b9c0f1b6a831c399e269772661

Input:  "abc"
Output: 900150983cd24fb0d6963f7d28e17f72

Input:  "message digest"
Output: f96b697d7cb7938d525a2f31aaf161d0

Input:  "abcdefghijklmnopqrstuvwxyz"
Output: c3fcd3d76192e4007dfb496cca67e13b

Input:  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"
Output: d174ab98d277d9f5a5611c2c9f419d9f

Input:  "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
Output: 57edf4a42c3d9b1d186b8495f32f161d
```

#### 6.1.2 Unit Tests - File Handling

- Read empty file
- Read small text file (< 1 KB)
- Read large file (> 10 MB)
- Read binary files with null bytes
- Handle file permission errors
- Handle missing files gracefully
- Handle directory paths (should error)

#### 6.1.3 Integration Tests

- Text input → hash computation → correct output
- File input → hash computation → verification
- stdin input → hash computation → verification
- Command-line argument parsing → correct mode selection
- Multiple sequential operations without state contamination

#### 6.1.4 TUI Tests

- Menu navigation (all transitions valid)
- Text input validation
- File browser functionality
- Results display accuracy
- Error dialog display and acknowledgment
- Terminal resize handling
- Color support detection

### 6.2 Test Fixtures

Create test data files:

```
tests/fixtures/
├── empty.txt                (0 bytes)
├── single_byte.bin          (1 byte)
├── small_text.txt           (100 bytes)
├── medium_text.txt          (1 MB)
├── large_binary.bin         (10 MB)
├── unicode_text.txt         (UTF-8 encoded)
└── null_bytes.bin           (contains null bytes)
```

### 6.3 Test Execution Framework

Use **Unity** or **cmocka** testing framework.

```bash
# Run all tests
make test

# Run specific test suite
make test-md5
make test-file-handler
make test-tui

# Run with coverage reporting
make test-coverage

# Run with memory leak detection (valgrind)
make test-valgrind
```

### 6.4 Performance Testing

```
Test Case: Large File Hash (100 MB)
Target: < 500 ms on modern hardware
Measurement: Time to compute hash, verify against known value

Test Case: Memory Usage
Target: < 10 MB peak memory for any file size
Measurement: Peak resident memory (using /usr/bin/time or similar)

Test Case: TUI Responsiveness
Target: < 50 ms input latency
Measurement: Time between keypress and screen update
```

---

## 7. Build and Compilation

### 7.1 Build System

Primary build tool: **GNU Make**

### 7.2 Compiler Flags

```makefile
CFLAGS = -std=c99 -Wall -Wextra -pedantic -O2
DEBUG_FLAGS = -g -O0 -DDEBUG
SANITIZE_FLAGS = -fsanitize=address -fsanitize=undefined
```

### 7.3 Dependencies

**Required:**
- C99-compatible compiler (gcc, clang)
- GNU Make
- POSIX-compliant system (Linux, macOS)

**Optional:**
- ncurses library (for TUI)
- xclip/pbcopy (for clipboard support)
- Doxygen (for documentation generation)

### 7.4 Build Targets

```makefile
make              # Build release binary
make debug        # Build debug binary
make clean        # Remove build artifacts
make test         # Build and run tests
make coverage     # Generate coverage report
make install      # Install binary to system
make uninstall    # Remove installed binary
make docs         # Generate Doxygen documentation
```

### 7.5 Compilation Output

```bash
$ make
gcc -std=c99 -Wall -Wextra -pedantic -O2 -c src/md5.c -o obj/md5.o
gcc -std=c99 -Wall -Wextra -pedantic -O2 -c src/file_handler.c -o obj/file_handler.o
gcc -std=c99 -Wall -Wextra -pedantic -O2 -c src/tui.c -o obj/tui.o
gcc -std=c99 -Wall -Wextra -pedantic -O2 -c src/utils.c -o obj/utils.o
gcc -std=c99 -Wall -Wextra -pedantic -O2 -c src/main.c -o obj/main.o
gcc -o md5 obj/md5.o obj/file_handler.o obj/tui.o obj/utils.o obj/main.o -lncurses
```

---

## 8. Error Handling

### 8.1 Error Codes

```c
#define MD5_SUCCESS            0
#define MD5_ERROR_FILE_OPEN    1
#define MD5_ERROR_FILE_READ    2
#define MD5_ERROR_INVALID_ARG  3
#define MD5_ERROR_MEMORY       4
#define MD5_ERROR_TUI_INIT     5
#define MD5_ERROR_PERMISSION   6
```

### 8.2 Error Messages

All errors should be descriptive and actionable:

```
Error: Cannot open file 'document.txt': Permission denied
Error: Invalid input string: exceeds maximum length of 65536 bytes
Error: Terminal size too small (minimum 80x24 required)
Error: Failed to initialize ncurses library
```

### 8.3 Graceful Degradation

- TUI falls back to CLI mode if terminal is unavailable
- Clipboard operations are optional (program continues without them)
- Color support is detected and disabled if unavailable

---

## 9. Security Considerations

### 9.1 Input Validation

- Validate all file paths for directory traversal attacks
- Limit input string length to prevent buffer overflows
- Check file size before reading to prevent resource exhaustion

### 9.2 Memory Safety

- Use `memset()` to clear sensitive data (hash buffers)
- Implement buffer overflow protections
- Use address sanitizer during development

### 9.3 File Permissions

- Verify read permissions before hashing files
- Do not follow symbolic links by default
- Provide option to verify file integrity

---

## 10. Documentation Requirements

### 10.1 Code Documentation

- Doxygen-compatible comments for all public functions
- Clear parameter and return value documentation
- Algorithm complexity annotations (O(n) for hash, O(1) for state)

### 10.2 User Documentation

**README.md:** Project overview, quick start
**USAGE.md:** Detailed usage examples
**BUILD.md:** Compilation and installation instructions
**RFC1321.md:** MD5 algorithm reference

### 10.3 Example Usage

```bash
# Hash a text string
$ md5 -t "Hello, World!"
65a8e27d8d55e25962f1db4e5e9c1b6e

# Hash a file
$ md5 -f document.txt
5d41402abc4b2a76b9719d911017c592

# Hash from stdin
$ echo "test" | md5 -s

# Quiet output (hash only)
$ md5 -q -t "test"
098f6bcd4621d373cade4e832627b4f6

# Verbose output
$ md5 -v -f image.png
MD5 Hash Implementation v1.0
File: /home/user/image.png
File Size: 2097152 bytes
Processing Time: 45.234 ms
Blocks Processed: 4096
MD5 Digest: abc123def456...

# Interactive TUI mode
$ md5
[Launches interactive menu interface]
```

---

## 11. Version Control and Delivery

### 11.1 Repository Structure

```
main branch          # Production-ready code
├── develop         # Development branch
├── feature/*       # Feature branches
├── bugfix/*        # Bug fix branches
└── release/*       # Release branches
```

### 11.2 Release Versioning

Follow Semantic Versioning (SemVer):
- **v1.0.0** - Initial release with core MD5, CLI, and basic TUI
- **v1.1.0** - Enhanced TUI features, clipboard support
- **v2.0.0** - Additional hash algorithms (SHA1, SHA256)

### 11.3 Deliverables

- Source code with comprehensive documentation
- Compiled binaries for Linux and macOS
- Test suite with coverage > 90%
- User manual and API documentation
- Example scripts and use cases

---

## 12. Performance and Optimization

### 12.1 Performance Targets

| Operation | Target | Acceptance |
|-----------|--------|-----------|
| Hash 1 MB file | < 10 ms | < 50 ms |
| Hash 1 GB file | < 10 seconds | < 30 seconds |
| TUI responsiveness | < 50 ms | < 200 ms |
| Memory footprint | < 1 MB | < 10 MB |

### 12.2 Optimization Strategies

- Use buffered I/O for large files (64 KB buffers)
- Implement loop unrolling for MD5 rounds
- Cache frequently used constants
- Use efficient string formatting for hex encoding

---

## 13. Future Enhancements

### 13.1 Potential Additions

- Support for additional hash algorithms (SHA1, SHA256, SHA512)
- Batch file hashing (directory traversal)
- Hash comparison mode (verify files)
- Configuration file support (.md5rc)
- Internationalization (i18n) support
- Web UI interface using libmicrohttpd

### 13.2 Platform Expansion

- Windows native build (MinGW)
- Mobile platforms (Android via JNI)
- Embedded systems (cross-compilation support)

---

## 14. Acceptance Criteria

The implementation is considered complete when:

- [ ] All RFC 1321 test vectors pass
- [ ] Program handles all input modes correctly (text, file, stdin)
- [ ] TUI is functional and responsive on 80x24+ terminals
- [ ] Command-line interface works as specified
- [ ] Test coverage exceeds 90%
- [ ] No memory leaks detected with valgrind
- [ ] Documentation is complete and accurate
- [ ] Program builds cleanly on Linux and macOS
- [ ] Performance targets are met
- [ ] All error cases are handled gracefully

---

## 15. Timeline and Milestones

| Phase | Duration | Deliverables |
|-------|----------|--------------|
| Phase 1: Core MD5 Algorithm | 1 week | md5.c, md5.h, test suite |
| Phase 2: File Handling | 3 days | file_handler.c, tests |
| Phase 3: CLI Interface | 3 days | main.c, argument parsing |
| Phase 4: TUI Development | 1 week | tui.c, ncurses integration |
| Phase 5: Integration & Testing | 1 week | Full system testing |
| Phase 6: Documentation | 3 days | Complete documentation |
| **Total** | **4 weeks** | Release v1.0.0 |

---

## 16. References

- [RFC 1321 - The MD5 Message-Digest Algorithm](https://tools.ietf.org/html/rfc1321)
- [NIST FIPS 180-4 - Secure Hash Standard](https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.180-4.pdf)
- [ncurses Programming Guide](https://tldp.org/HOWTO/NCURSES-Programming-HOWTO/)
- [C99 Standard](https://en.wikipedia.org/wiki/C99)
- [GNU Make Manual](https://www.gnu.org/software/make/manual/)

---

**Document Version:** 1.0  
**Last Updated:** 2026-06-28  
**Status:** Active  
**Prepared by:** SDCHESNEY
