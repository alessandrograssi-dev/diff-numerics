// NumericDiff.hpp
// -------------------------------------------------------------
// This header defines the NumericDiff class, which provides functionality
// for comparing two numerical data files line by line, with configurable
// tolerance, threshold, and output options.
//
// The class is used by the diff-numerics command-line tool and in tests.
// -------------------------------------------------------------

#pragma once
#include <cstdint>
#include <fstream>
#include <set>
#include <string>
#include <vector>

#include "Printer.hpp"

namespace numdiff {

/**
 * Configuration options for numerical comparison
 * Contains all user-configurable parameters for file comparison
 */
struct NumericDiffOptions {
    bool side_by_side = false;           // Display output in side-by-side format
    double tolerance = 1E-2;             // Percentage difference threshold (default 1%)
    double threshold = 1E-6;             // Absolute value threshold for near-zero values
    std::string comment_prefix = "#";    // Prefix for comment lines to ignore
    bool suppress_common_lines = false;  // Hide matching lines in output
    bool only_equal = false;             // Only report if files are equal/different
    bool quiet = false;                  // Suppress detailed output
    int line_length = 60;                // Maximum line length for output formatting
    bool color_diff_digits = false;      // Colorize only differing digits (not entire numbers)
    std::set<size_t> columns_to_compare; // Specific columns to compare (1-based, empty = all)
    std::string file1, file2;            // Paths to files being compared
};

/**
 * Results from a numerical comparison operation
 * Contains statistics about differences found between files
 */
struct NumericDiffResult {
    std::uint32_t n_different_lines = 0;  // Count of lines with differences
    double max_percentage_err = 0;        // Maximum percentage error found
};

/**
 * Main class for performing numerical comparison between two data files
 * 
 * Compares files line-by-line, token-by-token, treating numerical values specially.
 * Non-comment lines are tokenized, and numeric tokens are compared with configurable
 * tolerance and threshold settings. Results can be printed in various formats.
 */
class NumericDiff {
   public:
    NumericDiff() = delete;  // No default constructor
    
    /** Construct with options, using stdout for output */
    explicit NumericDiff(const NumericDiffOptions& opts);
    
    /** Construct with options and custom output stream */
    explicit NumericDiff(const NumericDiffOptions& opts, std::ostream& os)
        : options_(opts), printer_(os) {};
    
    /** Execute the comparison and return results */
    NumericDiffResult run();

   private:
    NumericDiffOptions options_;           // Comparison configuration
    static constexpr double big = 1.0E99;  // Value for "infinite" percentage difference
    Printer printer_;                      // Handles formatted output

   private:
    /** Compare two lines token-by-token, returns (has_diff, max_percentage_error) */
    std::pair<bool, double> compare_lines(const std::string& line1, const std::string& line2);
    
    /** Calculate percentage difference between two numeric values */
    double percentage_difference(double value1, double value2) const;
    
    /** Open a file and throw exception if it fails */
    std::ifstream open_and_validate_file(const std::string& file_path) const;
};
}  // namespace numdiff