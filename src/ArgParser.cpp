// ArgParser.cpp
// -------------------------------------------------------------
// Implementation of command-line argument parsing for diff-numerics
//
// Parses argc/argv, validates options, and handles help/version flags.
// Throws runtime_error with descriptive messages for invalid inputs.
// -------------------------------------------------------------

#include "ArgParser.hpp"

#include <iostream>
#include <sstream>

// Define the static usage/help text
const std::string ArgParser::usage =
    "Usage: diff-numerics [options] file1 file2\n"
    "Options:\n"
    "  -y,  --side-by-side             Show files side by side (default: off)\n"
    "  -ys, --suppress-common-lines    Suppress lines that are the same (implies side-by-side, "
    "default: off)\n"
    "  -t,  --tolerance <value>        Set tolerance for numeric comparison (default: 1e-2)\n"
    "  -T,  --threshold <value>        Set threshold for reporting differences (default: 1e-6)\n"
    "  -c,  --comment-string <prefix>  Set comment string (default: #)\n"
    "  -w,  --single-column-width <n>  Set maximum line length (default: 60)\n"
    "  -s,  --report-identical-files   Only show equal lines (default: off)\n"
    "  -q,  --quiet                    Suppress output (default: off)\n"
    "  -d,  --color-different-digits   Color differing digits (default: off)\n"
    "  -C,  --columns <list>           Compare only specified columns (comma-separated, 1-based, "
    "default: all)\n"
    "  -v,  --version                  Show program version and exit\n"
    "  -h,  --help                     Show this help message\n";

#ifndef NUMERIC_DIFF_VERSION
#define NUMERIC_DIFF_VERSION "v1.0.0"
#endif

/**
 * Print the usage text to stdout
 * Called when user passes -h/--help or when argument parsing fails
 */
void ArgParser::print_usage() {
    std::cout << usage << std::endl;
}

/**
 * Parse comma-separated column specification into a set of column indices
 * 
 * Input format: "1,3,5" -> {1, 3, 5}
 * Columns are 1-based (first column is 1, not 0)
 * Throws runtime_error if any column number is less than 1
 */
void ArgParser::parse_columns(const std::string& col_arg, std::set<size_t>& columns_to_compare) {
    std::stringstream ss(col_arg);
    std::string col;
    // Split on commas and parse each column number
    while (std::getline(ss, col, ',')) {
        size_t col_num = std::stoul(col);
        // Validate: columns are 1-based, cannot be 0
        if (col_num < 1) {
            throw std::runtime_error("Error: Column numbers must be at least 1 (got " +
                                     std::to_string(col_num) + ").");
        }
        columns_to_compare.insert(col_num);
    }
}

/**
 * Main argument parsing function
 * 
 * Iterates through argv, recognizing flags and their values.
 * Supports both short (-y) and long (--side-by-side) option formats.
 * First two non-option arguments are treated as file1 and file2.
 * 
 * Special handling:
 * - -v/--version: prints version and exits immediately
 * - -h/--help: handled by caller (not here)
 * - -ys: implies both suppress_common_lines and side_by_side
 * 
 * Throws runtime_error for:
 * - Unknown options
 * - Missing required values for flags
 * - Extra arguments beyond file1 and file2
 */
numdiff::NumericDiffOptions ArgParser::parse_args(int argc, char* argv[]) {
    numdiff::NumericDiffOptions o;  // Default-initialized options  // Default-initialized options
    
    // Iterate through all command-line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        // Version flag: print and exit immediately
        if (arg == "-v" || arg == "--version") {
            std::cout << "numeric-diff version " << NUMERIC_DIFF_VERSION << std::endl;
            std::exit(0);
        }
        // Side-by-side output format
        else if (arg == "-y" || arg == "--side-by-side") {
            o.side_by_side = true;
        }
        // Suppress common lines (implies side-by-side)
        else if (arg == "-ys" || arg == "--suppress-common-lines") {
            o.suppress_common_lines = true;
            o.side_by_side = true;  // Automatically enable side-by-side  // Automatically enable side-by-side
        }
        // Tolerance: percentage difference threshold
        else if ((arg == "-t" || arg == "--tolerance")) {
            if (i + 1 < argc) {
                o.tolerance = std::stod(argv[++i]);
            } else {
                throw std::runtime_error("Error: Missing value for " + arg + " option.");
            }
        }
        // Threshold: absolute value for near-zero comparison
        else if (arg == "-threshold" || arg == "-T" || arg == "--threshold") {
            if (i + 1 < argc) {
                o.threshold = std::stod(argv[++i]);
            } else {
                throw std::runtime_error("Error: Missing value for " + arg + " option.");
            }
        }
        // Comment prefix: lines starting with this are ignored
        else if ((arg == "--comment" || arg == "-c" || arg == "--comment-string")) {
            if (i + 1 < argc) {
                o.comment_prefix = argv[++i];
            } else {
                throw std::runtime_error("Error: Missing value for " + arg + " option.");
            }
        }
        // Line length: maximum characters per line in output
        else if (arg == "-w" || arg == "--single-column-width") {
            if (i + 1 < argc) {
                o.line_length = std::atoi(argv[++i]);
            } else {
                throw std::runtime_error("Error: Missing value for " + arg + " option.");
            }
        }
        // Only report whether files are equal (don't print differences)
        else if (arg == "--only-equal" || arg == "-s") {
            o.only_equal = true;
        }
        // Quiet mode: minimal output
        else if (arg == "-q" || arg == "--quiet") {
            o.quiet = true;
        }
        // Fine-grained colorization: only highlight differing digits
        else if (arg == "-d" || arg == "--color-different-digits") {
            o.color_diff_digits = true;
        }
        // Column selection: only compare specified columns (1-based)
        else if (arg == "-C" || arg == "--columns") {
            if (i + 1 < argc) {
                ArgParser::parse_columns(argv[++i], o.columns_to_compare);
            } else {
                throw std::runtime_error("Error: Missing value for " + arg + " option.");
            }
        }
        // First non-option argument: file1
        else if (o.file1.empty()) {
            o.file1 = arg;
        }
        // Second non-option argument: file2
        else if (o.file2.empty()) {
            o.file2 = arg;
        }
        // More than two files: error
        else {
            throw std::runtime_error("Unknown or extra argument: " + arg);
        }
    }
    
    // Validate all options before returning
    validate_options(o);
    return o;
}

/**
 * Validate parsed options for correctness and consistency
 * 
 * Checks:
 * - Both file paths are specified
 * - File paths are different
 * - Column width is within valid range [10, 200]
 * - Tolerance is within valid range [1e-15, 1e+3]
 * - Threshold is within valid range [0, 1e+3]
 * 
 * Throws runtime_error with descriptive message if validation fails
 */
void ArgParser::validate_options(const numdiff::NumericDiffOptions& o) {
    // Require both input files
    if (o.file1.empty() || o.file2.empty())
        throw std::runtime_error("Error: Two input files must be specified.");

    // Files must be different (comparing a file to itself is not useful)
    if (o.file1 == o.file2)
        throw std::runtime_error("Error: The two input files must be different.");

    // Validate line length: must be reasonable for display
    if (o.line_length < min_col_width || o.line_length > max_col_width)
        throw std::runtime_error("Error: Column width (" + std::to_string(o.line_length) +
                                 ") must be between " + std::to_string(min_col_width) + " and " +
                                 std::to_string(max_col_width) + ".");

    // Validate tolerance: very tight (1e-15) to very loose (1e+3)
    if (o.tolerance < min_tol || o.tolerance > max_tol)
        throw std::runtime_error("Error: Tolerance (" + std::to_string(o.tolerance) +
                                 ") must be between " + std::to_string(min_tol) + " and " +
                                 std::to_string(max_tol) + ".");

    // Validate threshold: non-negative, reasonable upper bound
    if (o.threshold < min_threshold || o.threshold > max_threshold)
        throw std::runtime_error("Error: Threshold (" + std::to_string(o.threshold) +
                                 ") must be between " + std::to_string(min_threshold) + " and " +
                                 std::to_string(max_threshold) + ".");
}

/**
 * Public entry point for parsing command-line arguments
 * 
 * Simply delegates to parse_args, which performs all parsing and validation.
 */
numdiff::NumericDiffOptions ArgParser::parse(int argc, char* argv[]) {
    return parse_args(argc, argv);
}