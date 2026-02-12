// NumericDiff.cpp
// -------------------------------------------------------------
#include "NumericDiff.hpp"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <sstream>
#include <vector>

#include "Formatter.hpp"
#include "Printer.hpp"
#include "TextParser.hpp"

namespace numdiff {

// Constructor: initialize with options and stdout printer
NumericDiff::NumericDiff(const NumericDiffOptions& opts) : options_(opts), printer_(std::cout) {}

// Open a file for reading and validate that it opened successfully
std::ifstream NumericDiff::open_and_validate_file(const std::string& file_path) const {
    std::ifstream fs(file_path);
    if (!fs) throw std::runtime_error("Error: could not open file: " + file_path);
    return fs;
}

/**
 * Main comparison algorithm
 * 
 * Reads both files line-by-line, skipping comment lines, and compares
 * corresponding non-comment lines. Accumulates statistics about differences.
 * Throws an exception if files have different numbers of non-comment lines.
 */
NumericDiffResult NumericDiff::run() {
    NumericDiffResult result;
    std::ifstream fs1 = open_and_validate_file(options_.file1);
    std::ifstream fs2 = open_and_validate_file(options_.file2);

    std::string line1, line2;
    bool file1_has_line = true, file2_has_line = true;
    
    // Main comparison loop: read and compare non-comment lines from both files
    while (file1_has_line && file2_has_line) {
        // Advance file1 to next non-comment line (or EOF)
        while (file1_has_line) {
            if (!std::getline(fs1, line1)) {
                file1_has_line = false;
                line1.clear();
                break;
            }
            if (options_.comment_prefix.empty() ||
                !TextParser::line_is_comment(line1, options_.comment_prefix))
                break;
        }
        
        // Advance file2 to next non-comment line (or EOF)
        while (file2_has_line) {
            if (!std::getline(fs2, line2)) {
                file2_has_line = false;
                line2.clear();
                break;
            }
            if (options_.comment_prefix.empty() ||
                !TextParser::line_is_comment(line2, options_.comment_prefix))
                break;
        }

        // Both files reached EOF simultaneously - normal exit
        if (!file1_has_line && !file2_has_line) break;

        // Compare the current pair of non-comment lines
        auto [is_diff, perc_err] = compare_lines(line1, line2);
        if (is_diff) {
            result.n_different_lines++;
            if (perc_err > result.max_percentage_err) result.max_percentage_err = perc_err;
        }
    }

    // Verify that file1 has no remaining non-comment lines
    while (std::getline(fs1, line1)) {
        if (!options_.comment_prefix.empty() &&
            TextParser::line_is_comment(line1, options_.comment_prefix))
            continue;
        if (compare_lines(line1, "").second > 0.0)
            throw std::runtime_error("Error: compare line on empty line resulted wrong.");
    }

    // Verify that file2 has no remaining non-comment lines
    while (std::getline(fs2, line2)) {
        if (!options_.comment_prefix.empty() &&
            TextParser::line_is_comment(line2, options_.comment_prefix))
            continue;
        if (compare_lines("", line2).second > 0.0)
            throw std::runtime_error("Error: compare line on empty line resulted wrong.");
    }
    return result;
}

/**
 * Compare two lines token-by-token
 * 
 * Tokenizes both lines, compares corresponding tokens, and formats output based on options.
 * Numeric tokens are compared with tolerance/threshold; non-numeric tokens are copied.
 * Returns a pair: (has_differences, max_percentage_error_in_line)
 */
std::pair<bool, double> NumericDiff::compare_lines(const std::string& line1,
                                                   const std::string& line2) {
    // Split lines into whitespace-separated tokens
    std::vector<std::string> tokens1 = TextParser::tokenize(line1);
    std::vector<std::string> tokens2 = TextParser::tokenize(line2);

    // Require same number of columns in both lines
    if (tokens1.size() != tokens2.size()) throw std::runtime_error("Column count mismatch");

    // Storage for formatted output tokens and error messages
    std::vector<std::string> output1, output2, errors;
    std::vector<bool> is_diff;
    std::string toPrint1, toPrint2, toPrintErrors;
    std::pair<bool, double> res{};  // Return value: (has_diff, max_error)

    // Calculate column widths for aligned output
    std::vector<size_t> col_widths = Formatter::calculate_col_widths(tokens1, tokens2);
    size_t n = col_widths.size();

    bool any_error = false;                // Track if any differences found on this line
    double max_diff_this_line = 0.0;       // Maximum percentage diff on this line
    
    // Compare each column/token pair
    for (size_t i = 0; i < n; ++i) {
        // Skip columns not in the comparison set (if specified)
        if (!options_.columns_to_compare.empty() && options_.columns_to_compare.count(i + 1) == 0) {
            continue;  // Column filtering: skip this column entirely
        }
        
        // Numeric comparison: both tokens must be parseable as numbers
        if (TextParser::string_is_numeric(tokens1[i]) &&
            TextParser::string_is_numeric(tokens2[i])) {
            double v1 = std::stod(tokens1[i]);
            double v2 = std::stod(tokens2[i]);
            double diff = percentage_difference(v1, v2);
            
            // Check if difference exceeds tolerance
            if (std::abs(diff) > options_.tolerance) {
                any_error = true;
                if (std::abs(diff) > max_diff_this_line) max_diff_this_line = std::abs(diff);
                
                // Apply color formatting to highlight differences
                std::string& t1 = tokens1[i];
                std::string& t2 = tokens2[i];
                if (options_.color_diff_digits) {
                    Formatter::colorize_different_digits(t1, t2);  // Colorize only differing digits
                } else {
                    Formatter::make_red(t1);  // Colorize entire numbers
                    Formatter::make_red(t2);
                }
                output1.push_back(t1);
                output2.push_back(t2);
                is_diff.push_back(true);
                
                // Format the percentage error for display
                std::ostringstream oss;
                oss << std::setw(static_cast<int>(col_widths[i])) << std::setfill(' ') << std::right
                    << diff << "%";
                errors.push_back(oss.str());
            } else {
                // Within tolerance: no difference
                output1.push_back(tokens1[i]);
                output2.push_back(tokens2[i]);
                is_diff.push_back(false);
                errors.push_back(std::string(col_widths[i], ' '));
            }
        } else {
            // Non-numeric comparison: just copy tokens verbatim (no comparison)
            output1.push_back(tokens1[i]);
            output2.push_back(tokens2[i]);
            is_diff.push_back(false);
            errors.push_back(std::string(col_widths[i], ' '));
        }
    }

    // Set return value based on whether differences were found
    if (any_error) {
        res.first = true;
        res.second = max_diff_this_line;
    }

    // Skip printing if only_equal mode (just accumulate statistics)
    if (options_.only_equal) {
        return res;
    }

    // Format and print the comparison results
    if (options_.side_by_side) {
        // Side-by-side format: columns aligned horizontally
        if (options_.suppress_common_lines) {
            if (any_error) {
                printer_.print_side_by_side_tokens(output1, output2, col_widths,
                                                   options_.line_length);
            }
            // else: suppress common lines (print nothing)
        } else {
            // Print all lines, both matching and differing
            printer_.print_side_by_side_tokens(output1, output2, col_widths, options_.line_length);
        }
    } else {
        // Traditional diff format: < line1 / > line2 / >> errors
        // Join tokens back into complete lines for output
        auto join = [](const std::vector<std::string>& v) {
            std::string out;
            for (size_t i = 0; i < v.size(); ++i) {
                if (i > 0) out += " ";
                out += v[i];
            }
            return out;
        };
        toPrint1 = join(output1);
        toPrint2 = join(output2);
        toPrintErrors = join(errors);
        printer_.print_diff(toPrint1, toPrint2, toPrintErrors);
    }
    return res;
}

/**
 * Calculate percentage difference between two numeric values
 * 
 * Handles special cases:
 * - Both near zero: returns 0 (no difference)
 * - One near zero, other not: returns "big" (infinite difference)
 * - Otherwise: returns |v1-v2| / max(|v1|,|v2|) * 100
 */
double NumericDiff::percentage_difference(double value1, double value2) const {
    // Both values near zero: treat as equal
    if (std::abs(value1) < options_.threshold && std::abs(value2) < options_.threshold) {
        return 0.0;
    }
    
    // One value near zero, other significant: treat as max difference
    if ((std::abs(value1) < options_.threshold && std::abs(value2) >= options_.threshold) ||
        (std::abs(value2) < options_.threshold && std::abs(value1) >= options_.threshold)) {
        return big;
    }
    
    // Calculate relative percentage difference
    double percentage_diff =
        std::abs(value1 - value2) / std::max(std::abs(value1), std::abs(value2)) * 100.0;
    
    // Return 0 if within tolerance (treat as equal)
    if (percentage_diff < options_.tolerance) {
        return 0.0;
    }
    
    return percentage_diff;
}

}  // namespace numdiff