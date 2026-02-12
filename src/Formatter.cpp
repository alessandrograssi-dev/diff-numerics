#include "Formatter.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <charconv>

std::vector<size_t> Formatter::calculate_col_widths(const std::vector<std::string>& t1,
                                        const std::vector<std::string>& t2) {
    size_t n = std::min(t1.size(), t2.size());
    std::vector<size_t> col_widths(n, 0);
    for (size_t i = 0; i < n; ++i) 
        col_widths[i] = std::max(t1[i].size(), t2[i].size());
    return col_widths;
}

// Helper to strip ANSI escape codes (color codes) from a string
std::string Formatter::strip_ansi(const std::string& input) {
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

// Ensure the string ends with the ANSI reset code if the last color set is not reset
void Formatter::ensure_ansi_reset(std::string& str) {
    // Only handle red for now: \033[31m ... \033[0m
    // If the string contains a red start but does not end with reset, add reset
    const std::string red_start = std::string(RED);
    const std::string reset = std::string(RESET);
    size_t last_red = str.rfind(red_start);
    size_t last_reset = str.rfind(reset);
    if (last_red != std::string::npos &&
        (last_reset == std::string::npos || last_reset < last_red)) {
        str += reset;
    }
}


// Extract the first n visible (non-ANSI) characters from a string, preserving formatting codes
std::string Formatter::extract_visible_prefix(const std::string& input, size_t n) {
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
    ensure_ansi_reset(result);
    return result;
}


// Colorize only the digits that differ between s1 and s2 (ANSI red: \033[31m ... \033[0m)
void Formatter::colorize_different_digits(std::string& s1, std::string& s2) {
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
        out1 = mant1.substr(0, diff_start) + std::string(RED) + mant1.substr(diff_start) + std::string(RESET);
    } else {
        out1 = mant1;
    }
    if (diff_start < mant2.size()) {
        out2 = mant2.substr(0, diff_start) + std::string(RED) + mant2.substr(diff_start) + std::string(RESET);
    } else {
        out2 = mant2;
    }
    // If any difference in mantissa, color all exponent (even if equal)
    if (!exp1.empty() || !exp2.empty()) {
        if (diff_start < n || mant1.size() != mant2.size()) {
            if (!exp1.empty()) out1 += std::string(RED) + exp1 + std::string(RESET);
            if (!exp2.empty()) out2 += std::string(RED) + exp2 + std::string(RESET);
        } else if (exp1 == exp2) {
            out1 += exp1;
            out2 += exp2;
        } else {
            if (!exp1.empty()) out1 += std::string(RED) + exp1 + std::string(RESET);
            if (!exp2.empty()) out2 += std::string(RED) + exp2 + std::string(RESET);
        }
    }
    s1 = out1;
    s2 = out2;
}
