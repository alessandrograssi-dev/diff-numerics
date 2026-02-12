// Printer.cpp
// -------------------------------------------------------------
// Implementation of output formatting for diff-numerics
//
// Handles all formatted output including summary statistics,
// side-by-side aligned display, and unified diff format.
// Properly handles ANSI color codes in width calculations.
// -------------------------------------------------------------

#include "Printer.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>

#include "Formatter.hpp"
#include "NumericDiff.hpp"

/**
 * Print summary or detailed comparison results
 * 
 * Handles three output modes:
 * 
 * 1. Quiet mode: Only print if files differ, show brief summary
 * 2. Only-equal mode: Print whether files match or differ with statistics
 * 3. Normal mode: No special handling here (line-by-line output handled
 *    by NumericDiff::compare_lines calling print_diff/print_side_by_side_tokens)
 * 
 * This method is called after all line comparisons are complete.
 */
void Printer::print(const numdiff::NumericDiffResult& result,
                    const numdiff::NumericDiffOptions& opts) {
    // Quiet mode: suppress all output if files match
    if (opts.quiet) {
        if (result.n_different_lines == 0) {
            return;  // Files match, print nothing
        } else {
            // Files differ: print brief summary
            os_ << "Comparing " << opts.file1 << " and " << opts.file2 << "\n";
            os_ << "Tolerance: " << opts.tolerance << ", Threshold: " << opts.threshold << "\n";
            os_ << "Files DIFFER: " << result.n_different_lines
                << " lines differ, max percentage error: " << result.max_percentage_err << "%\n";
        }
        return;
    }

    // Only-equal mode: just report whether files match or differ
    if (opts.only_equal) {
        os_ << "Comparing " << opts.file1 << " and " << opts.file1 << "\n";
        os_ << "Tolerance: " << opts.tolerance << ", Threshold: " << opts.threshold << "\n";
        if (result.n_different_lines == 0) {
            os_ << "Files are EQUAL within tolerance.\n";
            return;
        } else {
            os_ << "Files DIFFER: " << result.n_different_lines
                << " lines differ, max percentage error: " << result.max_percentage_err << "%\n";
        }
        return;
    }
}

/**
 * Print tokens side-by-side with column alignment
 * 
 * Algorithm:
 * 1. Build two strings (one per file) with aligned columns
 * 2. Calculate padding for each column (excluding ANSI codes from width)
 * 3. Choose separator based on whether differences exist (red color present)
 * 4. Truncate both lines to line_length visible characters
 * 5. Print with separator between them
 * 
 * Column width expansion: If a token is longer than the specified column
 * width, the column expands to fit it (values are never truncated).
 */
void Printer::print_side_by_side_tokens(const std::vector<std::string>& tokens1,
                                        const std::vector<std::string>& tokens2,
                                        const std::vector<size_t>& col_widths, int line_length) {
    std::ostringstream oss1, oss2;  // Build output strings for both files
    size_t ncols = std::max(tokens1.size(), tokens2.size());
    
    // Build each column with proper padding
    for (size_t i = 0; i < ncols; ++i) {
        // Get tokens for this column (empty string if column doesn't exist)
        std::string t1 = (i < tokens1.size()) ? tokens1[i] : "";
        std::string t2 = (i < tokens2.size()) ? tokens2[i] : "";
        
        // Strip ANSI codes to get actual visible width
        std::string t1_stripped = Formatter::strip_ansi(t1);
        std::string t2_stripped = Formatter::strip_ansi(t2);
        
        // Determine column width: max of specified width and actual token widths
        // This ensures values are never truncated
        size_t colw = (i < col_widths.size()) ? col_widths[i] : static_cast<size_t>(line_length);
        colw = std::max({colw, t1_stripped.size(), t2_stripped.size()});
        
        // Add token to first file's output with padding
        oss1 << t1;
        if (t1_stripped.size() < colw) oss1 << std::string(colw - t1_stripped.size(), ' ');
        
        // Add token to second file's output with padding
        oss2 << t2;
        if (t2_stripped.size() < colw) oss2 << std::string(colw - t2_stripped.size(), ' ');
        
        // Add space between columns (except after last column)
        if (i + 1 < ncols) {
            oss1 << " ";
            oss2 << " ";
        }
    }
    
    std::string l1 = oss1.str();
    std::string l2 = oss2.str();
    
    // Choose separator based on whether differences exist
    // Red color indicates differences
    bool has_red = (Formatter::string_is_red(l1) || Formatter::string_is_red(l2));
    const char* sep = has_red ? "   |   " : "       ";  // | for diffs, spaces for matches
    
    // Truncate lines to specified visible length (preserving ANSI codes)
    l1 = Formatter::extract_visible_prefix(l1, static_cast<size_t>(line_length));
    l2 = Formatter::extract_visible_prefix(l2, static_cast<size_t>(line_length));
    
    // Print: file1_line   separator   file2_line
    os_ << l1 << sep << l2 << "\n";
}

/**
 * Print differences in unified diff format
 * 
 * Format:
 *   < line_from_file1
 *   > line_from_file2
 *   >> percentage_errors
 * 
 * Only prints if differences exist (detected by presence of red color).
 * This format is traditional diff-style, familiar to unix users.
 */
void Printer::print_diff(const std::string& output1, const std::string& output2,
                         const std::string& errors) {
    // Check if either line contains differences (has red highlighting)
    bool has_red1 = Formatter::string_is_red(output1);
    bool has_red2 = Formatter::string_is_red(output2);
    
    // Only print if differences exist
    if (has_red1 || has_red2) {
        os_ << '\n';                       // Blank line before diff block
        os_ << "< " << output1 << "\n";   // File 1 line
        os_ << "> " << output2 << "\n";   // File 2 line
        os_ << ">>" << errors << "\n";     // Percentage errors
    }
}