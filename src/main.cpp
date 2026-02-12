// main.cpp
// -------------------------------------------------------------
// This is the main entry point for the diff-numerics command-line tool.
// It parses command-line arguments, configures options, and runs the
// NumericDiff class to compare two numerical data files.
//
// Usage and options are printed if arguments are missing or invalid.
// -------------------------------------------------------------

#include <string>

#include "NumericDiff.hpp"
#include "NumericDiffOption.hpp"

int main(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            NumericDiffOption::print_usage();
            return 0;
        }
    }
    NumericDiffOption opts;
    if (!opts.parse(argc, argv)) return 1;
    if (!opts.validate()) return 1;

    NumericDiff diff(opts);
    return diff.run();
}