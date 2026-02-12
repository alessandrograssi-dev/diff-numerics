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

struct NumericDiffOptions {
    // Option values
    bool side_by_side = false;
    double tolerance = 1E-2;
    double threshold = 1E-6;
    std::string comment_prefix = "#";
    bool suppress_common_lines = false;
    bool only_equal = false;
    bool quiet = false;
    int line_length = 60;
    bool color_diff_digits = false;
    std::set<size_t> columns_to_compare;
    std::string file1, file2;
};

struct NumericDiffResult {
    std::uint32_t n_different_lines = 0;
    double max_percentage_err = 0;
};

class NumericDiff {
   public:
    NumericDiff() = delete;
    explicit NumericDiff(const NumericDiffOptions& opts);
    explicit NumericDiff(const NumericDiffOptions& opts, std::ostream& os)
        : options_(opts), printer_(os) {};
    NumericDiffResult run();

   private:
    NumericDiffOptions options_;
    static constexpr double big = 1.0E99;
    Printer printer_;

   private:
    // Compare two lines and print results
    std::pair<bool, double> compare_lines(const std::string& line1, const std::string& line2);
    // Compute percentage difference between two values
    double percentage_difference(double value1, double value2) const;
    std::ifstream open_and_validate_file(const std::string& file_path) const;
};
}  // namespace numdiff