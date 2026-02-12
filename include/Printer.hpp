// Printer.hpp
// -------------------------------------------------------------
// Output formatting and printing for diff-numerics
//
// Provides methods to format and print comparison results in various
// styles (side-by-side, unified diff) with proper alignment and
// colorization. Handles both summary output and detailed line-by-line
// comparison display.
// -------------------------------------------------------------

#pragma once
#include <ostream>
#include <string>
#include <vector>

// Forward declarations to avoid circular dependency
namespace numdiff {
struct NumericDiffResult;
struct NumericDiffOptions;
}  // namespace numdiff

/**
 * Output formatter for numerical comparison results
 * 
 * Handles all output formatting for diff-numerics, including:
 * - Summary statistics (files equal/differ, error counts)
 * - Side-by-side aligned column output
 * - Unified diff format (< line1 / > line2 / >> errors)
 * - Proper handling of ANSI color codes in width calculations
 * 
 * Outputs to a configurable ostream (typically stdout, but can be
 * redirected for testing or file output).
 */
class Printer {
   public:
    /** Construct printer with output stream */
    Printer(std::ostream& os) : os_(os) {}

    /**
     * Print summary or detailed results based on options
     * 
     * Behavior depends on options:
     * - quiet mode: only print if files differ, show summary
     * - only_equal mode: print whether files are equal/differ
     * - normal mode: detailed line-by-line output (handled elsewhere)
     */
    void print(const numdiff::NumericDiffResult& result, const numdiff::NumericDiffOptions& opts);

    /**
     * Print tokens in side-by-side format with column alignment
     * 
     * Formats two rows of tokens (columns) for parallel display:
     * - Calculates padding based on column widths
     * - Handles ANSI color codes (excluded from width calculations)
     * - Uses '|' separator for differing lines, spaces for matching
     * - Truncates output to line_length visible characters
     */
    void print_side_by_side_tokens(const std::vector<std::string>& tokens1,
                                   const std::vector<std::string>& tokens2,
                                   const std::vector<size_t>& col_widths, int line_length);

    /**
     * Print differences in unified diff format
     * 
     * Format:
     *   < line_from_file1
     *   > line_from_file2
     *   >> percentage_errors
     * 
     * Only prints if at least one line contains differences (has red color).
     */
    void print_diff(const std::string& output1, const std::string& output2,
                    const std::string& errors);

   private:
    std::ostream& os_;  // Output stream (stdout or custom)
};