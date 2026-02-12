#pragma once
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

class Printer {
public:
    Printer() = delete;

    // New helper: print tokens side by side, column by column, with color and padding
    static void print_side_by_side_tokens(const std::vector<std::string>& tokens1,
                                        const std::vector<std::string>& tokens2,
                                        const std::vector<size_t>& col_widths,
                                        int line_length);

    // Print differences in a diff-like format
    static void print_diff(const std::string& output1, const std::string& output2,
                            const std::string& errors);
};