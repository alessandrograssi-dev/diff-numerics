// NumericDiff.hpp
// -------------------------------------------------------------
// This header defines the NumericDiff class, which provides functionality
// for comparing two numerical data files line by line, with configurable
// tolerance, threshold, and output options.
//
// The class is used by the diff-numerics command-line tool and in tests.
// -------------------------------------------------------------

#pragma once
#include <set>
#include <string>
#include <vector>

#include "NumericDiffOption.hpp"

class NumericDiff {
   public:
    // Constructor: set up comparison options and file paths
    explicit NumericDiff(const NumericDiffOption& opts);
    // Run the comparison and print results according to options and returns the number of differing
    // lines or -1 if an error occurred (e.g., file not found)
    int run();

   private:
    // File paths and options
    std::string file1_;
    std::string file2_;
    double tol_;
    double threshold_;
    bool side_by_side_;
    std::string comment_char_;
    int line_length_;
    // suppress_common_lines_ only suppresses common lines if explicitly set (not by -y/side_by_side
    // alone)
    bool suppress_common_lines_;
    bool only_equal_;
    bool quiet_;
    bool color_diff_digits_ = false;
    std::set<size_t> columns_to_compare_;

   private:
    // Helper: count columns in a file
    uint filesColumns(const std::string& file) const;
    // Helper: check if a line is a comment
    inline bool isLineComment(const std::string& line) const {
        size_t pos = line.find_first_not_of(" \t");
        if (pos == std::string::npos) return false;
        return line.compare(pos, comment_char_.size(), comment_char_) == 0;
    }
    // Helper: check if a line contains a comment but is not just a comment
    inline bool lineContainsComment_but_isNotJustComment(const std::string& line) const {
        size_t pos = line.find(comment_char_);
        return pos != std::string::npos && pos != 0 && !line.substr(0, pos).empty();
    }
    // Helper: remove comment from a line
    inline std::string removeComment(const std::string& line) const {
        size_t pos = line.find(comment_char_);
        return (pos == std::string::npos) ? line : line.substr(0, pos);
    }
    // Helper: check if a string is numeric
    bool isNumeric(const std::string& str) const;
    // Compare two lines and print results
    void compareLine(const std::string& line1, const std::string& line2) const;
    // Compute percentage difference between two values
    double percentageDifference(double value1, double value2) const;
    // Compare two values (not used directly)
    void compareValues(double value1, double value2) const;
    // Print diff output in various formats
    void printDiff(const std::string& line1, const std::string& line2,
                   const std::string& errors) const;
    // Print side-by-side tokens, aligning columns. Never truncate or cut numeric values: if a value
    // is longer than the max column width, the column expands to fit the value. The max column
    // width only limits padding/alignment, not the content of the numbers. ANSI color codes are
    // ignored for width calculations.
    void printSideBySideTokens(const std::vector<std::string>& tokens1,
                               const std::vector<std::string>& tokens2,
                               const std::vector<size_t>& col_widths) const;
    // Print a string in red (for errors)
    inline void printRed(std::string& str) const {
        // Placeholder for red text output, e.g., using ANSI escape codes
        str = "\033[31m" + str + "\033[0m";  // Red color
    }
    // Remove ANSI color codes from a string
    std::string stripAnsi(const std::string& input) const;
    // Extract the first n visible (non-ANSI) characters from a string, preserving formatting codes
    std::string extractVisiblePrefix(const std::string& input, size_t n) const;
    // Ensure the string ends with the ANSI reset code if the last color set is not reset
    void ensureAnsiReset(std::string& str) const;
    // Helper: colorize only the differing digits between two numeric strings
    void colorizeDiffDigits(std::string& s1, std::string& s2) const;

    // For summary/statistics
    mutable size_t diff_lines_ = 0;
    mutable double max_percentage_error_ = 0.0;
};
