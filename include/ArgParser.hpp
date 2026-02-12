#pragma once
#include "NumericDiff.hpp"

class ArgParser {
public:
	ArgParser() = delete;
    static void print_usage();
    static numdiff::NumericDiffOption parse(int argc, char* argv[]);
	
private:
	static numdiff::NumericDiffOption parse_args(int argc, char *argv[]);
	static void validate_options(const numdiff::NumericDiffOption& options);
	static void parse_columns(const std::string& col_arg, std::set<size_t>& columns_to_compare);
	static const std::string usage;

	static constexpr int min_col_width = 10;
	static constexpr int max_col_width = 200;
	static constexpr double min_tol = 1e-15;
	static constexpr double max_tol = 1e+3;
	static constexpr double min_threshold = 0.0;
	static constexpr double max_threshold = 1e+3;
};