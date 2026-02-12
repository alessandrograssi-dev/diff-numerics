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
#include "ArgParser.hpp"
#include <iostream>

using namespace numdiff;

int main(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            ArgParser::print_usage();
            return 0;
        }
    }
    NumericDiffOption opts;
    try {
        opts = ArgParser::parse(argc, argv);
    } catch (const std::runtime_error& e) {
        std::cout << e.what();
        ArgParser::print_usage();
        return -1;
    }
    
    NumericDiff diff(opts);
    
    NumericDiffResult r;
    try {
        r = diff.run();
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << '\n';
        return -1;
    }

    if (opts.quiet) {
        // Print nothing if files are equal, otherwise print as normal (with all options except
        // quiet)
        if (r.n_different_lines == 0) {
            return 0;
        } else {
            // Print summary as in only_equal_ mode
            std::cout << "Comparing " << opts.file1 << " and " << opts.file2 << "\n";
            std::cout << "Tolerance: " << opts.tolerance << ", Threshold: " << opts.threshold << "\n";
            std::cout << "Files DIFFER: " << r.n_different_lines
                    << " lines differ, max percentage error: " << r.max_percentage_err << "%\n";
        }
        return 0;
    }

    if (opts.only_equal) {
        std::cout << "Comparing " << opts.file1 << " and " << opts.file1 << "\n";
        std::cout << "Tolerance: " << opts.tolerance << ", Threshold: " << opts.threshold << "\n";
        if (r.n_different_lines == 0) {
            std::cout << "Files are EQUAL within tolerance.\n";
            return 0;
        } else {
            std::cout << "Files DIFFER: " << r.n_different_lines
                    << " lines differ, max percentage error: " << r.max_percentage_err << "%\n";
        }
        return 0;
    }

    return 0;
}