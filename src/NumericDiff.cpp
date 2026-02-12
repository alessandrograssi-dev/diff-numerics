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
#include "Printer.hpp"
#include "TextParser.hpp"
#include "Formatter.hpp"

#include <sstream>
#include <iomanip>
#include <fstream>
#include <set>
#include <vector>

namespace numdiff {
    // Constructor: initialize from options struct
    NumericDiff::NumericDiff(const NumericDiffOption& opts)
        : options_(opts){
        TextParser::set_comment_prefix(opts.comment_prefix);
    }

    std::ifstream NumericDiff::open_and_validate_file(const std::string& file_path) const {
        std::ifstream fs(file_path);
        if (!fs) 
            throw std::runtime_error("Error: could not open file: " + file_path);
        return fs;
    }

    // Main entry: run the comparison and print results
    NumericDiffResult NumericDiff::run() {
        NumericDiffResult result;
        std::ifstream fs1 = open_and_validate_file(options_.file1);
        std::ifstream fs2 = open_and_validate_file(options_.file2);
        
        std::string line1, line2;
        bool file1_has_line = true, file2_has_line = true;
        while (file1_has_line && file2_has_line) {
            // Advance file1 to next non-comment line
            while (file1_has_line) {
                if (!std::getline(fs1, line1)) {
                    file1_has_line = false;
                    line1.clear();
                    break;
                }
                if (options_.comment_prefix.empty() || !TextParser::line_is_comment(line1)) 
                    break;
            }
            // Advance file2 to next non-comment line
            while (file2_has_line) {
                if (!std::getline(fs2, line2)) {
                    file2_has_line = false;
                    line2.clear();
                    break;
                }
                if (options_.comment_prefix.empty() || !TextParser::line_is_comment(line2)) 
                    break;
            }

            if (!file1_has_line && !file2_has_line) 
                break;

            auto [is_diff, perc_err] = compare_lines(line1, line2);
            if (is_diff) {
                result.n_different_lines++;
                if (perc_err > result.max_percentage_err)
                    result.max_percentage_err = perc_err;
            }
        }


        while (std::getline(fs1, line1)) {
            if (!options_.comment_prefix.empty() && TextParser::line_is_comment(line1)) 
                continue;
            if (compare_lines(line1, "").second > 0.0)
                throw std::runtime_error("Error: compare line on empty line resulted wrong.");
        }

        while (std::getline(fs2, line2)) {
            if (!options_.comment_prefix.empty() && TextParser::line_is_comment(line2)) 
                continue;
            if (compare_lines("", line2).second > 0.0)
                throw std::runtime_error("Error: compare line on empty line resulted wrong.");
        }
        return result;
    }

    // Compare two lines, print differences according to options
    std::pair<bool, double> NumericDiff::compare_lines(const std::string& line1, const std::string& line2) const {
        // Tokenize both lines
        std::vector<std::string> tokens1 = TextParser::tokenize(line1);
        std::vector<std::string> tokens2 = TextParser::tokenize(line2);
        
        if (tokens1.size() != tokens2.size())
            throw std::runtime_error("Column count mismatch");

        std::vector<std::string> output1, output2, errors;
        std::vector<bool> is_diff;
        std::string toPrint1, toPrint2, toPrintErrors;
        std::pair<bool, double> res{};

        // Calculate column widths for pretty output
        std::vector<size_t> col_widths = Formatter::calculate_col_widths(tokens1, tokens2);
        size_t n = col_widths.size();

        bool any_error = false;
        double max_diff_this_line = 0.0;
        for (size_t i = 0; i < n; ++i) {
            if (!options_.columns_to_compare.empty() && options_.columns_to_compare.count(i + 1) == 0) {
                // Skip this column entirely (do not print)
                continue;
            }
            // Compare only if both tokens are numeric
            if (TextParser::string_is_numeric(tokens1[i]) && TextParser::string_is_numeric(tokens2[i])) {
                double v1 = std::stod(tokens1[i]);
                double v2 = std::stod(tokens2[i]);
                double diff = percentage_difference(v1, v2);
                if (std::abs(diff) > options_.tolerance) {
                    any_error = true;
                    if (std::abs(diff) > max_diff_this_line) max_diff_this_line = std::abs(diff);
                    std::string& t1 = tokens1[i];
                    std::string& t2 = tokens2[i];
                    if (options_.color_diff_digits) {
                        Formatter::colorize_different_digits(t1, t2);
                    } else {
                        Formatter::make_red(t1);
                        Formatter::make_red(t2);
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
                    Printer::print_side_by_side_tokens(output1, output2, col_widths, options_.line_length);
                }
                // else: do not print common lines
            } else {
                Printer::print_side_by_side_tokens(output1, output2, col_widths, options_.line_length);
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
            Printer::print_diff(toPrint1, toPrint2, toPrintErrors);
        }
        return res;
    }

    // Calculate the percentage difference between two values
    double NumericDiff::percentage_difference(double value1, double value2) const {
        if (std::abs(value1) < options_.threshold && std::abs(value2) < options_.threshold) {
            return 0.0;
        }
        if ((std::abs(value1) < options_.threshold && std::abs(value2) >= options_.threshold) ||
            (std::abs(value2) < options_.threshold && std::abs(value1) >= options_.threshold)) {
            return big;  // One value is below threshold, the other is not
        }
        double percentage_diff =
            std::abs(value1 - value2) / std::max(std::abs(value1), std::abs(value2)) * 100.0;
        if (percentage_diff < options_.tolerance) {
            return 0.0;  // Values are within tolerance
        }
        return percentage_diff;  // Return the percentage difference
    }

}