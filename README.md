# diff-numerics

A high-performance numerical file comparison tool with intelligent tolerance handling and fine-grained difference visualization.

## 📋 Overview

`diff-numerics` is a specialized command-line tool for comparing numerical data files with configurable tolerance and threshold settings. Unlike traditional text-based diff tools, it understands numerical values and can report differences that exceed specified tolerance levels while ignoring insignificant variations.

Built with modern C++17, the project emphasizes clean architecture, comprehensive testing, and production-ready code quality suitable for scientific computing, data validation, and automated testing pipelines.

## ✨ Key Features

### Numerical Comparison
- **Smart tolerance handling**: Configurable relative tolerance and absolute threshold for near-zero values
- **Scientific notation support**: Handles standard and exponential notation (e.g., `1.23e-5`)
- **Column-selective comparison**: Compare only specified columns with 1-based indexing

### Output Formats
- **Side-by-side view**: Aligned columns with intelligent separator selection
- **Unified diff format**: Traditional `< file1` / `> file2` / `>> errors` style
- **Multiple modes**: Quiet mode, summary mode, or detailed line-by-line output

### Visualization
- **Fine-grained colorization**: Digit-level highlighting shows exactly where numbers differ
- **Full highlighting**: Red coloring for entire differing values
- **ANSI-aware formatting**: Proper handling of terminal escape sequences in width calculations

### Flexibility
- **Comment handling**: Skip lines starting with configurable prefixes
- **Suppress common lines**: Show only differences in side-by-side mode
- **Configurable output width**: Control line length for terminal display

## 🔧 Technical Highlights

### Modern C++17 Implementation
- Extensive use of structured bindings, `std::from_chars`, and STL algorithms
- Zero-cost abstractions with compile-time optimization
- RAII principles and careful resource management throughout

### Clean Architecture
- Modular design with distinct responsibilities: parsing, formatting, comparison, output
- Stateless utility classes for thread-safe text processing and formatting
- Namespace organization preventing naming collisions
- Forward declarations avoiding circular dependencies

### Code Quality
- Comprehensive inline documentation explaining algorithms and edge cases
- Consistent coding style and professional naming conventions
- Robust error handling with descriptive messages
- Full test suite using GoogleTest with automated CI via CTest

### Performance Considerations
- High-performance numeric parsing using `std::from_chars` (locale-independent, no exceptions)
- Efficient ANSI escape sequence processing for terminal output
- Minimal memory allocations in hot paths

---

## 🏗️ Architecture

The project follows a modular, object-oriented design with clear separation of concerns:

```
diff-numerics/
├── include/              # Public headers
│   ├── NumericDiff.hpp   # Core comparison engine and result structures
│   ├── ArgParser.hpp     # Command-line argument parser
│   ├── Printer.hpp       # Output formatter (side-by-side, unified diff)
│   ├── Formatter.hpp     # ANSI code handling, string formatting
│   ├── TextParser.hpp    # Tokenization, comment detection, numeric validation
│   └── ...
├── src/                  # Implementation files
│   ├── main.cpp          # Entry point
│   ├── NumericDiff.cpp   # Comparison algorithm
│   ├── ArgParser.cpp     # CLI parsing and validation
│   ├── Printer.cpp       # Output rendering
│   ├── Formatter.cpp     # ANSI manipulation utilities
│   ├── TextParser.cpp    # Text parsing utilities
│   └── ...
└── test/                 # GoogleTest test suite
    └── test-diff-numerics.cpp
```

### Core Components

#### `NumericDiff` (Main Engine)
- Encapsulates comparison logic and file I/O
- Provides `NumericDiffOptions` for configuration and `NumericDiffResult` for statistics
- Implements line-by-line comparison with token-based numeric validation
- Handles special cases: near-zero values, scientific notation, column filtering

#### `ArgParser` (CLI Interface)
- Parses command-line arguments with robust validation
- Supports both short (`-t`) and long (`--tolerance`) option formats
- Validates ranges for all numeric parameters
- Provides comprehensive help text and version information

#### `Printer` (Output Formatter)
- Generates formatted output in multiple styles (quiet, only-equal, side-by-side, unified)
- Handles ANSI escape sequences in width calculations
- Intelligent separator selection based on line differences

#### `Formatter` (String Utilities)
- ANSI escape sequence manipulation (add, remove, validate)
- Column width calculation excluding formatting codes
- Fine-grained digit-level colorization for numeric strings
- Visible character extraction with format preservation

#### `TextParser` (Text Processing)
- Whitespace-based tokenization using `std::istringstream`
- Comment line detection with configurable prefixes
- High-performance numeric validation using `std::from_chars`

---

## 🚀 Building

### Prerequisites

- **C++17 compatible compiler** (GCC 7+, Clang 5+, MSVC 2017+)
- **CMake 3.10+**
- **Make** (or Ninja)

### Build Instructions

```bash
# Clone the repository
git clone <repository-url>
cd diff-numerics

# Create build directory and configure
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
make -j$(nproc)

# Run tests
make test

# Binary location
./build/bin/diff-numerics
```

### Build Targets

```bash
make all          # Build everything (default)
make diff-numerics # Build only the main executable
make test         # Build and run tests
make clean        # Clean build artifacts
```

### Installation

To install `diff-numerics` system-wide:

```bash
# From the build directory
sudo make install
```

This will install:
- Binary: `/usr/local/bin/diff-numerics`
- Man page: `/usr/local/share/man/man1/diff-numerics.1`

After installation, you can run `diff-numerics` from anywhere:

```bash
diff-numerics file1.dat file2.dat
```

To uninstall:

```bash
# From the build directory
sudo make uninstall
```

---

## 📖 Usage

### Basic Syntax

```bash
diff-numerics [options] file1 file2
```

### Common Options

| Option | Long Form | Description | Default |
|--------|-----------|-------------|---------|
| `-t` | `--tolerance` | Percentage difference threshold | `1e-2` (1%) |
| `-T` | `--threshold` | Absolute value for near-zero comparison | `1e-6` |
| `-y` | `--side-by-side` | Show files side by side | Off |
| `-ys` | `--suppress-common-lines` | Hide matching lines (implies `-y`) | Off |
| `-c` | `--comment-string` | Comment prefix to ignore | `#` |
| `-w` | `--single-column-width` | Maximum line length | `60` |
| `-s` | `--report-identical-files` | Only report if files equal/differ | Off |
| `-q` | `--quiet` | Suppress detailed output | Off |
| `-d` | `--color-different-digits` | Colorize only differing digits | Off |
| `-C` | `--columns` | Compare specific columns (1-based) | All columns |
| `-v` | `--version` | Show version and exit | - |
| `-h` | `--help` | Show help message | - |

### Examples

#### Basic comparison with default tolerance
```bash
diff-numerics data1.dat data2.dat
```

#### Side-by-side view with custom tolerance
```bash
diff-numerics -y -t 1e-5 data1.dat data2.dat
```

#### Compare only columns 1, 3, and 5
```bash
diff-numerics -C 1,3,5 data1.dat data2.dat
```

#### Suppress matching lines, show only differences
```bash
diff-numerics -ys data1.dat data2.dat
```

#### Fine-grained digit colorization
```bash
diff-numerics -y -d data1.dat data2.dat
```

#### Quiet mode (only report if files differ)
```bash
diff-numerics -q data1.dat data2.dat
echo $?  # Exit code: 0 if equal, non-zero if different
```

---

## 🔬 Algorithm Details

### Comparison Logic

1. **File Reading**: Both files are read line-by-line, skipping comment lines
2. **Tokenization**: Each line is split into whitespace-separated tokens
3. **Column Validation**: Both lines must have the same number of tokens
4. **Token Comparison**:
   - **Numeric tokens**: Compared using percentage difference formula
   - **Non-numeric tokens**: Copied verbatim (not compared)
5. **Percentage Calculation**:
   ```
   diff = |value1 - value2| / max(|value1|, |value2|) * 100
   ```
6. **Special Cases**:
   - Both values < threshold: treated as equal (0% difference)
   - One value < threshold, other ≥ threshold: infinite difference
7. **Output Formatting**: Results formatted based on options

### Tolerance vs Threshold

- **Tolerance** (`-t`): Relative percentage difference (e.g., 1% = 0.01)
  - Values differing by less than this percentage are considered equal
  - Applies to all comparisons where both values exceed the threshold

- **Threshold** (`-T`): Absolute value cutoff for near-zero comparisons
  - Values below this are treated as "effectively zero"
  - Prevents false positives when comparing very small numbers

**Example**:
```
tolerance = 1e-2 (1%)
threshold = 1e-6

Compare 1.0001 vs 1.0000:
  diff = |1.0001 - 1.0000| / max(1.0001, 1.0000) * 100 = 0.01%
  Result: EQUAL (0.01% < 1%)

Compare 1e-7 vs 2e-7:
  Both < threshold (1e-6)
  Result: EQUAL (both effectively zero)

Compare 1e-7 vs 1e-5:
  One < threshold, one > threshold
  Result: DIFFER (infinite difference)
```

---

## ✅ Testing

The project includes a comprehensive test suite built with GoogleTest.

### Running Tests

```bash
# Run all tests
make test

# Or run the test binary directly for detailed output
./build/bin/diff-numerics-tests
```

### Test Coverage

The test suite validates:
- Default and tight tolerance comparisons
- Side-by-side and unified diff output formats
- Common line suppression
- Quiet mode behavior
- Column-selective comparison
- Scientific notation handling
- Fine-grained digit colorization
- CLI argument validation (invalid options, missing files, etc.)
- Edge cases (same file, missing files, empty lines)

---

## 🛠️ Development

### Code Style

- **C++ Standard**: C++17
- **Naming Conventions**:
  - Classes: `PascalCase`
  - Functions/methods: `snake_case`
  - Member variables: `snake_case_` (trailing underscore)
  - Constants: `UPPER_CASE` or `snake_case` for `constexpr`
- **Documentation**: Doxygen-style comments for all public interfaces
- **Error Handling**: Exceptions for unrecoverable errors, return values for expected failures

### Adding New Features

1. Update relevant header in `include/`
2. Implement in corresponding `src/*.cpp` file
3. Add tests in `test/test-diff-numerics.cpp`
4. Update this README if user-facing
5. Run tests: `make test`

### Debugging

```bash
# Build with debug symbols
cmake .. -DCMAKE_BUILD_TYPE=Debug
make

# Run with debugger
gdb ./build/bin/diff-numerics
```

---

## 📄 License

See [LICENSE](LICENSE) file for details.

---

## 👤 Author

Alessandro Grassi

---

## 🤝 Contributing

Contributions are welcome! Please:
1. Fork the repository
2. Create a feature branch
3. Add tests for new functionality
4. Ensure all tests pass
5. Submit a pull request

---

## 📝 Version History

- **v1.0.0** (2026-02-12): Initial release with core functionality
  - Numerical comparison with tolerance/threshold
  - Side-by-side and unified diff formats
  - Fine-grained digit colorization
  - Column-selective comparison
  - Comprehensive test suite
