#include "ArgParser.hpp"

#include <iostream>
#include <sstream>

// Define static member
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

// Implementation of print_usage
void ArgParser::print_usage() {
    std::cout << usage << std::endl;
}

void ArgParser::parse_columns(const std::string& col_arg, std::set<size_t>& columns_to_compare) {
    std::stringstream ss(col_arg);
    std::string col;
    while (std::getline(ss, col, ',')) {
        size_t col_num = std::stoul(col);
        if (col_num < 1) {
            throw std::runtime_error("Error: Column numbers must be at least 1 (got " +
                                     std::to_string(col_num) + ").\n");
        }
        columns_to_compare.insert(col_num);
    }
}

numdiff::NumericDiffOption ArgParser::parse_args(int argc, char* argv[]) {
    numdiff::NumericDiffOption o;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-v" || arg == "--version") {
            std::cout << "numeric-diff version " << NUMERIC_DIFF_VERSION << std::endl;
            std::exit(0);
        } else if (arg == "-y" || arg == "--side-by-side") {
            o.side_by_side = true;
        } else if (arg == "-ys" || arg == "--suppress-common-lines") {
            o.suppress_common_lines = true;
            o.side_by_side = true;
        } else if ((arg == "-t" || arg == "--tolerance")) {
            if (i + 1 < argc) {
                o.tolerance = std::stof(argv[++i]);
            } else {
                throw std::runtime_error("Error: Missing value for " + arg + " option.\n");
            }
        } else if (arg == "-threshold" || arg == "-T" || arg == "--threshold") {
            if (i + 1 < argc) {
                o.threshold = std::stof(argv[++i]);
            } else {
                throw std::runtime_error("Error: Missing value for " + arg + " option.\n");
            }
        } else if ((arg == "--comment" || arg == "-c" || arg == "--comment-string")) {
            if (i + 1 < argc) {
                o.comment_prefix = argv[++i];
            } else {
                throw std::runtime_error("Error: Missing value for " + arg + " option.\n");
            }
        } else if (arg == "-w" || arg == "--single-column-width") {
            if (i + 1 < argc) {
                o.line_length = std::atoi(argv[++i]);
            } else {
                throw std::runtime_error("Error: Missing value for " + arg + " option.\n");
            }
        } else if (arg == "--only-equal" || arg == "-s") {
            o.only_equal = true;
        } else if (arg == "-q" || arg == "--quiet") {
            o.quiet = true;
        } else if (arg == "-d" || arg == "--color-different-digits") {
            o.color_diff_digits = true;
        } else if (arg == "-C" || arg == "--columns") {
            if (i + 1 < argc) {
                ArgParser::parse_columns(argv[++i], o.columns_to_compare);
            } else {
                throw std::runtime_error("Error: Missing value for " + arg + " option.\n");
            }
        } else if (o.file1.empty()) {
            o.file1 = arg;
        } else if (o.file2.empty()) {
            o.file2 = arg;
        } else {
            throw std::runtime_error("Unknown or extra argument: " + arg + "\n");
        }
    }
    validate_options(o);
    return o;
}

void ArgParser::validate_options(const numdiff::NumericDiffOption& o) {
    if (o.file1.empty() || o.file2.empty())
        throw std::runtime_error("Error: Two input files must be specified.\n");

    if (o.file1 == o.file2)
        throw std::runtime_error("Error: The two input files must be different.\n");

    if (o.line_length < min_col_width || o.line_length > max_col_width)
        throw std::runtime_error("Error: Column width (" + std::to_string(o.line_length) +
                                 ") must be between " + std::to_string(min_col_width) + " and " +
                                 std::to_string(max_col_width) + ".\n");

    if (o.tolerance < min_tol || o.tolerance > max_tol)
        throw std::runtime_error("Error: Tolerance (" + std::to_string(o.tolerance) +
                                 ") must be between " + std::to_string(min_tol) + " and " +
                                 std::to_string(max_tol) + ".\n");

    if (o.threshold < min_threshold || o.threshold > max_threshold)
        throw std::runtime_error("Error: Threshold (" + std::to_string(o.threshold) +
                                 ") must be between " + std::to_string(min_threshold) + " and " +
                                 std::to_string(max_threshold) + ".\n");
}

// Implement parse and validate as wrappers for parse_args and validate_options
numdiff::NumericDiffOption ArgParser::parse(int argc, char* argv[]) {
    return parse_args(argc, argv);
}