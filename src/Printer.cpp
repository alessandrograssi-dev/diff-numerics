#include "Printer.hpp"

#include "Formatter.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>

// New helper: print tokens side by side, column by column, with color and padding
void Printer::print_side_by_side_tokens(const std::vector<std::string>& tokens1,
                                      const std::vector<std::string>& tokens2,
                                      const std::vector<size_t>& col_widths,
                                      int line_length) {
    // Print tokens side by side, aligning columns. Never truncate or cut numeric values: if a value
    // is longer than the max column width, the column expands to fit the value. The max column
    // width only limits padding/alignment, not the content of the numbers. ANSI color codes are
    // ignored for width calculations.
    std::ostringstream oss1, oss2;
    size_t ncols = std::max(tokens1.size(), tokens2.size());
    for (size_t i = 0; i < ncols; ++i) {
        std::string t1 = (i < tokens1.size()) ? tokens1[i] : "";
        std::string t2 = (i < tokens2.size()) ? tokens2[i] : "";
        std::string t1_stripped = Formatter::strip_ansi(t1);
        std::string t2_stripped = Formatter::strip_ansi(t2);
        // Determine the width for this column: max of user col_width and actual token width
        size_t colw = (i < col_widths.size()) ? col_widths[i] : static_cast<size_t>(line_length);
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
    bool has_red = (Formatter::string_is_red(l1) || Formatter::string_is_red(l2));
    const char* sep = has_red ? "   |   " : "       ";
    l1 = Formatter::extract_visible_prefix(l1, static_cast<size_t>(line_length));
    l2 = Formatter::extract_visible_prefix(l2, static_cast<size_t>(line_length));
    std::cout << l1 << sep << l2 << "\n";
}


// Print differences in a diff-like format
void Printer::print_diff(const std::string& output1, const std::string& output2,
                            const std::string& errors) {
    // Only print lines that contain red marks (i.e., differences)
    bool has_red1 = Formatter::string_is_red(output1);
    bool has_red2 = Formatter::string_is_red(output2);
    if (has_red1 || has_red2) {
        std::cout << '\n';
        std::cout << "< " << output1 << "\n";
        std::cout << "> " << output2 << "\n";
        std::cout << ">>" << errors << "\n";
    }
}