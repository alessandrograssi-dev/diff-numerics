// ArgParser.hpp
// -------------------------------------------------------------
// Command-line argument parser for diff-numerics
//
// Provides static methods to parse command-line arguments, validate options,
// and generate usage/help text. All parsing is done statically without
// creating instances of this class.
// -------------------------------------------------------------

#pragma once
#include "NumericDiff.hpp"

/**
 * Static utility class for parsing and validating command-line arguments
 * 
 * Handles all command-line option parsing, validation, and help text generation.
 * This class cannot be instantiated (deleted default constructor).
 */
class ArgParser {
   public:
    ArgParser() = delete;  // No instances allowed
    
    /** Print usage/help information to stdout */
    static void print_usage();
    
    /** Parse command-line arguments and return validated options */
    static numdiff::NumericDiffOptions parse(int argc, char* argv[]);

   private:
    /** Internal parsing logic that processes argv and builds options struct */
    static numdiff::NumericDiffOptions parse_args(int argc, char* argv[]);
    
    /** Validate parsed options for consistency and valid ranges */
    static void validate_options(const numdiff::NumericDiffOptions& options);
    
    /** Parse comma-separated column list (e.g., "1,3,5") into a set */
    static void parse_columns(const std::string& col_arg, std::set<size_t>& columns_to_compare);
    
    /** Full usage/help text string */
    static const std::string usage;

    // Validation constraints for options
    static constexpr int min_col_width = 10;        // Minimum column width
    static constexpr int max_col_width = 200;       // Maximum column width
    static constexpr double min_tol = 1e-15;        // Minimum tolerance (very tight)
    static constexpr double max_tol = 1e+3;         // Maximum tolerance (very loose)
    static constexpr double min_threshold = 0.0;    // Minimum threshold (zero)
    static constexpr double max_threshold = 1e+3;   // Maximum threshold
};