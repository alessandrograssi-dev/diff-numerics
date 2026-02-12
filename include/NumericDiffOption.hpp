// NumericDiffOption.h
#pragma once
#include <set>
#include <string>

class NumericDiffOption {
   public:
    // Option values
    bool side_by_side = false;
    double tolerance = 1E-2;
    double threshold = 1E-6;
    std::string comment_char = "#";
    bool suppress_common_lines = false;
    bool only_equal = false;
    bool quiet = false;
    int line_length = 60;
    bool color_diff_digits = false;
    std::set<size_t> columns_to_compare;
    std::string file1, file2;

    NumericDiffOption() = default;
    bool parse_args(int argc, char* argv[]);
    bool validate_options() const;
    static bool parse_columns(const std::string& col_arg, std::set<size_t>& columns_to_compare,
                              const std::string& usage);
    static const std::string usage;
    static void print_usage();

    bool parse(int argc, char* argv[]);
    bool validate() const;
};
