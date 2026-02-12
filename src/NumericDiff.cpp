// NumericDiff.cpp
// -------------------------------------------------------------
// This file implements the NumericDiff class, which compares two numerical
// data files line by line, with configurable tolerance, threshold, and output
// options. It provides the core logic for the diff-numerics tool.
//
// Key features:
// - Compares files line by line, token by token
// - Supports side-by-side and diff-style output
// - Highlights differences above a given tolerance
// - Handles comments, column widths, and summary statistics
// -------------------------------------------------------------

#include "NumericDiff.hpp"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <vector>

namespace numdiff {
    // Constructor: initialize from options struct
    NumericDiff::NumericDiff(const NumericDiffOption& opts)
        : options_(opts) {}

    // Main entry: run the comparison and print results
    NumericDiffResult NumericDiff::run() {
        NumericDiffResult result;
        std::ifstream fin1(options_.file1), fin2(options_.file2);

        // Check if files exist before trying to open
        if (!fin1) 
            throw std::runtime_error("Error: could not open file: " + options_.file1);
        if (!fin2) 
            throw std::runtime_error("Error: could not open file: " + options_.file2);
        
        std::string line1, line2;
        size_t total_lines = 0;
        bool file1_has_line = true, file2_has_line = true;
        while (true) {
            // Advance file1 to next non-comment line
            while (file1_has_line) {
                if (!std::getline(fin1, line1)) {
                    file1_has_line = false;
                    line1.clear();
                    break;
                }
                if (options_.comment_prefix.empty() || !isLineComment(line1)) break;
            }
            // Advance file2 to next non-comment line
            while (file2_has_line) {
                if (!std::getline(fin2, line2)) {
                    file2_has_line = false;
                    line2.clear();
                    break;
                }
                if (options_.comment_prefix.empty() || !isLineComment(line2)) break;
            }
            if (!file1_has_line && !file2_has_line) break;
            ++total_lines;
            auto [is_diff, perc_err] = compareLine(line1, line2);
            if (is_diff) {
                result.n_different_lines++;
                if (perc_err > result.max_percentage_err)
                    result.max_percentage_err = perc_err;
            }
        }
        while (std::getline(fin1, line1)) {
            if (!options_.comment_prefix.empty() && isLineComment(line1)) continue;
            ++total_lines;
            if (compareLine(line1, "").second > 0.0)
                throw std::runtime_error("Error: compare line on empty line resulted wrong.");
        }
        while (std::getline(fin2, line2)) {
            if (!options_.comment_prefix.empty() && isLineComment(line2)) continue;
            ++total_lines;
            if (compareLine("", line2).second > 0.0)
                throw std::runtime_error("Error: compare line on empty line resulted wrong.");
        }
        return result;
    }

    // Helper: count columns in a file (used for formatting)
    size_t NumericDiff::FilesColumns(const std::string& file) const {
        std::ifstream fin(file);
        if (!fin.is_open()) return 0;
        std::string line;
        if (!std::getline(fin, line)) return 0;
        std::istringstream iss(line);
        std::string token;
        size_t count = 0;
        while (iss >> token) ++count;
        return count;
    }

    // Helper: check if a string is numeric
    bool NumericDiff::isNumeric(const std::string& str) const {
        char* end = nullptr;
        std::strtod(str.c_str(), &end);
        return end != str.c_str() && *end == '\0';
    }

    // Helper: tokenize a line into a vector of strings
    static std::vector<std::string> tokenize(const std::string& line) {
        std::istringstream iss(line);
        std::vector<std::string> tokens;
        std::string token;
        while (iss >> token) tokens.push_back(token);
        return tokens;
    }

    // Helper: calculate column widths for side-by-side output
    static std::vector<size_t> calc_col_widths(const std::vector<std::string>& t1,
                                            const std::vector<std::string>& t2) {
        size_t n = std::min(t1.size(), t2.size());
        std::vector<size_t> col_widths(n, 0);
        for (size_t i = 0; i < n; ++i) {
            col_widths[i] = std::max(t1[i].size(), t2[i].size());
        }
        return col_widths;
    }

    // Compare two lines, print differences according to options
    std::pair<bool, double> NumericDiff::compareLine(const std::string& line1, const std::string& line2) const {
        // Tokenize both lines
        std::vector<std::string> tokens1 = tokenize(line1);
        std::vector<std::string> tokens2 = tokenize(line2);
        std::vector<std::string> output1, output2, errors;
        std::vector<bool> is_diff;
        std::string toPrint1, toPrint2, toPrintErrors;
        std::pair<bool, double> res{};

        // Calculate column widths for pretty output
        std::vector<size_t> col_widths = calc_col_widths(tokens1, tokens2);
        size_t n = col_widths.size();

        bool any_error = false;
        double max_diff_this_line = 0.0;
        for (size_t i = 0; i < n; ++i) {
            if (!options_.columns_to_compare.empty() && options_.columns_to_compare.count(i + 1) == 0) {
                // Skip this column entirely (do not print)
                continue;
            }
            // Compare only if both tokens are numeric
            if (isNumeric(tokens1[i]) && isNumeric(tokens2[i])) {
                double v1 = std::stod(tokens1[i]);
                double v2 = std::stod(tokens2[i]);
                double diff = percentageDifference(v1, v2);
                if (std::abs(diff) > options_.tolerance) {
                    any_error = true;
                    if (std::abs(diff) > max_diff_this_line) max_diff_this_line = std::abs(diff);
                    std::string t1 = tokens1[i];
                    std::string t2 = tokens2[i];
                    if (options_.color_diff_digits) {
                        colorizeDiffDigits(t1, t2);
                    } else {
                        printRed(t1);
                        printRed(t2);
                    }
                    output1.push_back(t1);
                    output2.push_back(t2);
                    is_diff.push_back(true);
                    std::ostringstream oss;
                    oss << std::setw(static_cast<int>(col_widths[i])) << std::setfill(' ') << std::right
                        << diff << "%";
                    errors.push_back(oss.str());
                } else {
                    output1.push_back(tokens1[i]);
                    output2.push_back(tokens2[i]);
                    is_diff.push_back(false);
                    errors.push_back(std::string(col_widths[i], ' '));
                }
            } else {
                // Non-numeric tokens are just copied
                output1.push_back(tokens1[i]);
                output2.push_back(tokens2[i]);
                is_diff.push_back(false);
                errors.push_back(std::string(col_widths[i], ' '));
            }
        }

        if (any_error) {
            res.first  = true;
            res.second = max_diff_this_line;
        }

        if (options_.only_equal) {
            // Do not print anything for individual lines in only_equal_ mode
            return res;
        }

        // Print in diff style
        if (options_.side_by_side) {
            if (options_.suppress_common_lines) {
                if (any_error) {
                    // Print tokens side by side, column by column
                    printSideBySideTokens(output1, output2, col_widths);
                }
                // else: do not print common lines
            } else {
                printSideBySideTokens(output1, output2, col_widths);
            }
        } else {
            // Join tokens for output
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
            printDiff(toPrint1, toPrint2, toPrintErrors);
        }
        return res;
    }

    // New helper: print tokens side by side, column by column, with color and padding
    void NumericDiff::printSideBySideTokens(const std::vector<std::string>& tokens1,
                                            const std::vector<std::string>& tokens2,
                                            const std::vector<size_t>& col_widths) const {
        // Print tokens side by side, aligning columns. Never truncate or cut numeric values: if a value
        // is longer than the max column width, the column expands to fit the value. The max column
        // width only limits padding/alignment, not the content of the numbers. ANSI color codes are
        // ignored for width calculations.
        std::ostringstream oss1, oss2;
        size_t ncols = std::max(tokens1.size(), tokens2.size());
        for (size_t i = 0; i < ncols; ++i) {
            std::string t1 = (i < tokens1.size()) ? tokens1[i] : "";
            std::string t2 = (i < tokens2.size()) ? tokens2[i] : "";
            std::string t1_stripped = stripAnsi(t1);
            std::string t2_stripped = stripAnsi(t2);
            // Determine the width for this column: max of user col_width and actual token width
            size_t colw = (i < col_widths.size()) ? col_widths[i] : static_cast<size_t>(options_.line_length);
            colw = std::max({colw, t1_stripped.size(), t2_stripped.size()});
            // Print first token, padded
            oss1 << t1;
            if (t1_stripped.size() < colw) oss1 << std::string(colw - t1_stripped.size(), ' ');
            // Print second token, padded
            oss2 << t2;
            if (t2_stripped.size() < colw) oss2 << std::string(colw - t2_stripped.size(), ' ');
            if (i + 1 < ncols) {
                oss1 << " ";
                oss2 << " ";
            }
        }
        std::string l1 = oss1.str();
        std::string l2 = oss2.str();
        // Decide separator: if either line has red color, use |, else use spaces
        bool has_red =
            (l1.find("\033[31m") != std::string::npos) || (l2.find("\033[31m") != std::string::npos);
        const char* sep = has_red ? "   |   " : "       ";
        l1 = extractVisiblePrefix(l1, static_cast<size_t>(options_.line_length));
        l2 = extractVisiblePrefix(l2, static_cast<size_t>(options_.line_length));
        std::cout << l1 << sep << l2 << "\n";
    }

    // Calculate the percentage difference between two values
    double NumericDiff::percentageDifference(double value1, double value2) const {
        if (std::abs(value1) < options_.threshold && std::abs(value2) < options_.threshold) {
            return 0.0;
        }
        if ((std::abs(value1) < options_.threshold && std::abs(value2) >= options_.threshold) ||
            (std::abs(value2) < options_.threshold && std::abs(value1) >= options_.threshold)) {
            return 1.E99;  // One value is below threshold, the other is not
        }
        double percentage_diff =
            std::abs(value1 - value2) / std::max(std::abs(value1), std::abs(value2)) * 100.0;
        if (percentage_diff < options_.tolerance) {
            return 0.0;  // Values are within tolerance
        }
        return percentage_diff;  // Return the percentage difference
    }

    // Helper to strip ANSI escape codes (color codes) from a string
    std::string NumericDiff::stripAnsi(const std::string& input) const {
        std::string result;
        bool in_escape = false;
        for (size_t i = 0; i < input.size(); ++i) {
            if (!in_escape) {
                if (input[i] == '\033' && i + 1 < input.size() && input[i + 1] == '[') {
                    in_escape = true;
                } else {
                    result += input[i];
                }
            } else {
                // End of ANSI escape sequence is marked by 'm'
                if (input[i] == 'm') {
                    in_escape = false;
                }
            }
        }
        return result;
    }

    // Extract the first n visible (non-ANSI) characters from a string, preserving formatting codes
    std::string NumericDiff::extractVisiblePrefix(const std::string& input, size_t n) const {
        std::string result;
        size_t visible_count = 0;
        bool in_escape = false;
        for (size_t i = 0; i < input.size(); ++i) {
            if (!in_escape) {
                if (input[i] == '\033' && i + 1 < input.size() && input[i + 1] == '[') {
                    in_escape = true;
                    result += input[i];
                } else {
                    if (visible_count < n) {
                        result += input[i];
                        ++visible_count;
                    } else {
                        break;
                    }
                }
            } else {
                result += input[i];
                if (input[i] == 'm') {
                    in_escape = false;
                }
            }
        }
        // Ensure the result ends with a reset code if it was in an escape sequence
        ensureAnsiReset(result);
        return result;
    }

    // Print differences in a diff-like format
    void NumericDiff::printDiff(const std::string& output1, const std::string& output2,
                                const std::string& errors) const {
        // Only print lines that contain red marks (i.e., differences)
        bool has_red1 = output1.find("\033[31m") != std::string::npos;
        bool has_red2 = output2.find("\033[31m") != std::string::npos;
        if (has_red1 || has_red2) {
            std::cout << '\n';
            std::cout << "< " << output1 << "\n";
            std::cout << "> " << output2 << "\n";
            std::cout << ">>" << errors << "\n";
        }
    }

    // Ensure the string ends with the ANSI reset code if the last color set is not reset
    void NumericDiff::ensureAnsiReset(std::string& str) const {
        // Only handle red for now: \033[31m ... \033[0m
        // If the string contains a red start but does not end with reset, add reset
        const std::string red_start = "\033[31m";
        const std::string reset = "\033[0m";
        size_t last_red = str.rfind(red_start);
        size_t last_reset = str.rfind(reset);
        if (last_red != std::string::npos &&
            (last_reset == std::string::npos || last_reset < last_red)) {
            str += reset;
        }
    }

    // Colorize only the digits that differ between s1 and s2 (ANSI red: \033[31m ... \033[0m)
    void NumericDiff::colorizeDiffDigits(std::string& s1, std::string& s2) const {
        // If either string contains 'e' or 'E', split into mantissa and exponent
        auto split_exp = [](const std::string& s) -> std::pair<std::string, std::string> {
            size_t epos = s.find_first_of("eE");
            if (epos == std::string::npos) return {s, ""};
            return {s.substr(0, epos), s.substr(epos)};
        };
        auto [mant1, exp1] = split_exp(s1);
        auto [mant2, exp2] = split_exp(s2);

        std::string out1, out2;
        size_t n = std::min(mant1.size(), mant2.size());
        size_t diff_start = n;
        // Find first differing digit in mantissa
        for (size_t i = 0; i < n; ++i) {
            if (mant1[i] != mant2[i]) {
                diff_start = i;
                break;
            }
        }
        // If mantissas are different length, diff starts at min size
        if (mant1.size() != mant2.size()) {
            diff_start = std::min(diff_start, n);
        }
        // Build colored mantissas: everything after diff_start is red
        if (diff_start < mant1.size()) {
            out1 = mant1.substr(0, diff_start) + "\033[31m" + mant1.substr(diff_start) + "\033[0m";
        } else {
            out1 = mant1;
        }
        if (diff_start < mant2.size()) {
            out2 = mant2.substr(0, diff_start) + "\033[31m" + mant2.substr(diff_start) + "\033[0m";
        } else {
            out2 = mant2;
        }
        // If any difference in mantissa, color all exponent (even if equal)
        if (!exp1.empty() || !exp2.empty()) {
            if (diff_start < n || mant1.size() != mant2.size()) {
                if (!exp1.empty()) out1 += "\033[31m" + exp1 + "\033[0m";
                if (!exp2.empty()) out2 += "\033[31m" + exp2 + "\033[0m";
            } else if (exp1 == exp2) {
                out1 += exp1;
                out2 += exp2;
            } else {
                if (!exp1.empty()) out1 += "\033[31m" + exp1 + "\033[0m";
                if (!exp2.empty()) out2 += "\033[31m" + exp2 + "\033[0m";
            }
        }
        s1 = out1;
        s2 = out2;
    }
}