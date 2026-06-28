# MD5 Hash Implementation

A cross-platform command-line C program that implements the MD5 message-digest algorithm with an interactive Text User Interface (TUI).

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
![Language: C](https://img.shields.io/badge/Language-C99-blue.svg)
![Status: Active Development](https://img.shields.io/badge/Status-Active%20Development-brightgreen.svg)

## Features

- **Core MD5 Algorithm** - Full RFC 1321 compliant MD5 implementation
- **Multiple Input Modes**:
  - Raw text strings (command-line)
  - Text files (UTF-8 and ASCII)
  - Binary files (any format)
  - Standard input (stdin)
- **Interactive TUI** - Menu-driven interface with file browser and results display
- **Command-Line Interface** - Powerful CLI with multiple options and output formats
- **Cross-Platform** - Linux, macOS, and Windows (POSIX-compliant systems)
- **Performance** - Optimized for speed with efficient buffer management
- **Memory Safe** - Careful attention to buffer management and error handling
- **Comprehensive Testing** - RFC 1321 test vectors and extensive test suite

## Quick Start

### Installation

```bash
# Clone the repository
git clone https://github.com/SDCHESNEY/md5_implementation.git
cd md5_implementation

# Build the project
make

# (Optional) Install to system path
make install
```

### Basic Usage

```bash
# Interactive TUI mode (no arguments)
./md5

# Hash a text string
./md5 -t "Hello, World!"

# Hash a file
./md5 -f document.txt

# Hash a binary file
./md5 -b image.png

# Read from stdin
echo "test" | ./md5 -s

# Quiet output (hash only)
./md5 -q -t "test"

# Verbose output with details
./md5 -v -f largefile.iso
```

## Command-Line Options

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

## Interactive TUI Features

The interactive mode provides an intuitive menu-driven interface:

- **Main Menu** - Navigate between different input modes
- **Text Input** - Enter text to hash with multiline support
- **File Browser** - Browse and select files with real-time preview
- **Results Display** - View hash results with copy-to-clipboard support
- **Settings** - Configure program behavior
- **About** - View program information

### TUI Navigation

- **Arrow Keys** - Navigate through menus and file lists
- **Enter** - Select current option
- **Number Keys** - Quick selection (1-7 for main menu)
- **Ctrl+D** - Finish text input
- **Ctrl+C** - Exit program

## Output Examples

### Standard Output
```
MD5 Hash Result
===============
Input: hello.txt
Hash:  5d41402abc4b2a76b9719d911017c592
Size:  5 bytes
```

### Quiet Mode
```
5d41402abc4b2a76b9719d911017c592
```

### Verbose Mode
```
MD5 Hash Implementation v1.0
File: /path/to/file
File Size: 1024 bytes
Processing Time: 2.345 ms
Blocks Processed: 1
MD5 Digest: 5d41402abc4b2a76b9719d911017c592
```

## Project Structure

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
├── tech_spec.md             # Technical specification
├── README.md                # This file
└── LICENSE                  # MIT License
```

## Building from Source

### Requirements

- **Compiler**: GCC or Clang with C99 support
- **Build Tool**: GNU Make
- **System**: POSIX-compliant (Linux, macOS, or Windows with POSIX layer)
- **Dependencies**:
  - ncurses library (for TUI support)
  - Standard C library

### Build Targets

```bash
# Build release binary (optimized)
make

# Build debug binary (with symbols)
make debug

# Build and run tests
make test

# Generate code coverage report
make coverage

# Run tests with memory leak detection (requires valgrind)
make test-valgrind

# Clean build artifacts
make clean

# Install binary to system
make install

# Remove installed binary
make uninstall

# Generate API documentation
make docs
```

## Testing

The project includes comprehensive test coverage:

### Test Suites

```bash
# Run all tests
make test

# Run specific test suite
make test-md5
make test-file-handler
make test-tui

# Run RFC 1321 test vectors
make test-vectors
```

### Test Coverage

- **MD5 Algorithm**: RFC 1321 test vectors (8 test cases)
- **File Handling**: Empty, small, and large files; binary data
- **Integration Tests**: End-to-end workflows
- **TUI Tests**: Menu navigation, input handling, display accuracy

### Performance Tests

```bash
# Test large file hashing (100 MB)
make benchmark-large-file

# Test memory usage
make benchmark-memory
```

## Performance

| Operation | Target | Status |
|-----------|--------|--------|
| Hash 1 MB file | < 10 ms | ✓ |
| Hash 1 GB file | < 10 seconds | ✓ |
| TUI responsiveness | < 50 ms | ✓ |
| Memory footprint | < 1 MB | ✓ |

## RFC 1321 Compliance

This implementation fully conforms to RFC 1321 - The MD5 Message-Digest Algorithm:

- ✓ Proper state initialization (A, B, C, D)
- ✓ All 64 MD5 rounds with correct constants
- ✓ Correct message padding
- ✓ Proper length field encoding (little-endian)
- ✓ Test vector verification

**Test Vectors Verified:**
```
Input: ""
Output: d41d8cd98f00b204e9800998ecf8427e ✓

Input: "abc"
Output: 900150983cd24fb0d6963f7d28e17f72 ✓

Input: "message digest"
Output: f96b697d7cb7938d525a2f31aaf161d0 ✓

Input: "abcdefghijklmnopqrstuvwxyz"
Output: c3fcd3d76192e4007dfb496cca67e13b ✓
```

## Security Considerations

⚠️ **Important**: MD5 is cryptographically broken and should **not** be used for security-critical applications such as digital signatures or password hashing. Use SHA-256 or better for secure applications.

**Recommended Uses:**
- Integrity verification of non-critical data
- File deduplication
- Checksums in data transfer
- Educational purposes
- Legacy system compatibility

This implementation is suitable for these non-security purposes and provides reference MD5 functionality.

## Documentation

- **[BUILD.md](docs/BUILD.md)** - Detailed build and installation instructions
- **[USAGE.md](docs/USAGE.md)** - Comprehensive usage guide with examples
- **[tech_spec.md](tech_spec.md)** - Complete technical specification
- **[RFC1321.md](docs/RFC1321.md)** - MD5 algorithm reference (RFC 1321)

## Examples

### Example 1: Hash a File
```bash
$ ./md5 -f myfile.txt
MD5 Hash Result
===============
Input: myfile.txt
Hash:  5d41402abc4b2a76b9719d911017c592
Size:  5 bytes
```

### Example 2: Verify File Integrity
```bash
$ ./md5 -q -f largefile.iso
abc123def456...

# Compare with known hash
$ echo "abc123def456..." | diff -
```

### Example 3: Hash Multiple Files (bash)
```bash
for file in *.txt; do
  echo "$file: $(./md5 -q -f "$file")"
done
```

### Example 4: Using in Pipe
```bash
$ cat document.txt | ./md5 -s
MD5 Hash Result
===============
Input: stdin
Hash:  5d41402abc4b2a76b9719d911017c592
```

## Contributing

Contributions are welcome! Please follow these guidelines:

1. **Fork** the repository
2. **Create** a feature branch (`git checkout -b feature/amazing-feature`)
3. **Implement** your changes with tests
4. **Test** thoroughly (`make test`)
5. **Commit** with clear messages
6. **Push** to your branch
7. **Open** a Pull Request

### Code Standards

- Follow C99 standard
- Use meaningful variable names
- Add comments for complex logic
- Ensure all tests pass
- Run `make coverage` to verify coverage

## Version History

- **v1.0.0** (2026-06-28) - Initial release
  - Core MD5 algorithm implementation
  - CLI interface with multiple input modes
  - Interactive TUI with file browser
  - Comprehensive test suite
  - RFC 1321 compliance

## Roadmap

### Upcoming Features
- [ ] Additional hash algorithms (SHA1, SHA256, SHA512)
- [ ] Batch file hashing (directory traversal)
- [ ] Hash comparison/verification mode
- [ ] Configuration file support (.md5rc)
- [ ] Internationalization (i18n) support
- [ ] Web UI interface

### Platform Expansion
- [ ] Native Windows support (MinGW)
- [ ] macOS optimizations
- [ ] Embedded systems support
- [ ] Mobile platform support (Android, iOS)

## Performance Benchmarks

Run benchmarks on your system:

```bash
# Compile with optimizations
make clean && make CFLAGS="-O3"

# Generate benchmark data
dd if=/dev/zero of=benchmark_1mb.bin bs=1M count=1
dd if=/dev/zero of=benchmark_100mb.bin bs=1M count=100

# Run benchmarks
time ./md5 -f benchmark_1mb.bin
time ./md5 -f benchmark_100mb.bin
```

## FAQ

**Q: Why MD5 and not SHA-256?**
A: This is a reference implementation for educational purposes and legacy system compatibility. For security-critical applications, use SHA-256 or better. See the security section above.

**Q: Can I use this in production?**
A: Yes, for non-security purposes like file integrity verification and checksums. Not recommended for cryptographic security.

**Q: Does it work on Windows?**
A: Yes, with POSIX compatibility layer (MinGW, Cygwin, or WSL). Native Windows support planned for v1.1.0.

**Q: How large can files be?**
A: Limited only by available disk and system memory. Uses buffered I/O for constant memory footprint.

**Q: Is there a GUI version?**
A: The interactive TUI provides a menu-driven interface. A web UI is planned for future releases.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## References

- [RFC 1321 - The MD5 Message-Digest Algorithm](https://tools.ietf.org/html/rfc1321)
- [NIST FIPS 180-4 - Secure Hash Standard](https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.180-4.pdf)
- [ncurses Programming Guide](https://tldp.org/HOWTO/NCURSES-Programming-HOWTO/)
- [C99 Standard](https://en.wikipedia.org/wiki/C99)

## Support

For issues, questions, or suggestions:

- **Issues**: [GitHub Issues](https://github.com/SDCHESNEY/md5_implementation/issues)
- **Discussions**: [GitHub Discussions](https://github.com/SDCHESNEY/md5_implementation/discussions)
- **Email**: chesneysd@gmail.com

## Author

**SDCHESNEY** - Initial implementation and project maintenance

---

**Last Updated**: 2026-06-28  
**Version**: 1.0.0  
**Status**: Active Development
