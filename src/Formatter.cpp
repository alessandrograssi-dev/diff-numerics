// Formatter.cpp
// -------------------------------------------------------------
// Implementation of string formatting utilities for diff-numerics
//
// Handles ANSI escape sequence manipulation, width calculations,
// and selective colorization of differing digits in numeric strings.
// -------------------------------------------------------------

#include "Formatter.hpp"

#include <algorithm>
#include <charconv>

/**
 * Calculate the maximum width for each column position
 * 
 * Given two vectors of tokens (columns), returns a vector where each element
 * is the maximum length between corresponding tokens in t1 and t2.
 * Used for aligned column output.
 */
std::vector<size_t> Formatter::calculate_col_widths(const std::vector<std::string>& t1,
                                                    const std::vector<std::string>& t2) {
    size_t n = std::min(t1.size(), t2.size());
    std::vector<size_t> col_widths(n, 0);
    // For each column, take the maximum width between the two vectors
    for (size_t i = 0; i < n; ++i) col_widths[i] = std::max(t1[i].size(), t2[i].size());
    return col_widths;
}

/**
 * Remove all ANSI escape sequences from a string
 * 
 * ANSI escape sequences follow the pattern: ESC[...m where ESC is \033
 * This function strips all such sequences, returning only visible characters.
 * Used for calculating actual display width of colored strings.
 */
std::string Formatter::strip_ansi(const std::string& input) {
    std::string result;
    bool in_escape = false;  // Track if we're inside an escape sequence
    
    for (size_t i = 0; i < input.size(); ++i) {
        if (!in_escape) {
            // Check for start of ANSI sequence: ESC[
            if (input[i] == '\033' && i + 1 < input.size() && input[i + 1] == '[') {
                in_escape = true;
            } else {
                result += input[i];  // Regular character, keep it
            }
        } else {
            // Inside escape sequence: skip until we find 'm'
            if (input[i] == 'm') {
                in_escape = false;  // End of sequence
            }
        }
    }
    return result;
}

/**
 * Ensure a string ends with ANSI reset code if it contains unclosed color
 * 
 * If the string has a red color start (\033[31m) but doesn't end with a
 * reset code (\033[0m), appends the reset. Prevents color from bleeding
 * into subsequent terminal output.
 */
void Formatter::ensure_ansi_reset(std::string& str) {
    const std::string red_start = std::string(RED);
    const std::string reset = std::string(RESET);
    
    // Find the last occurrence of red start and reset
    size_t last_red = str.rfind(red_start);
    size_t last_reset = str.rfind(reset);
    
    // If there's a red start without a corresponding reset after it, add reset
    if (last_red != std::string::npos &&
        (last_reset == std::string::npos || last_reset < last_red)) {
        str += reset;
    }
}

/**
 * Extract first n visible characters from a string, preserving ANSI codes
 * 
 * Counts only visible (non-escape-sequence) characters toward the limit,
 * but includes all ANSI codes in the output to preserve formatting.
 * Useful for truncating strings to a display width without breaking colors.
 * Ensures the result ends with a reset code if needed.
 */
std::string Formatter::extract_visible_prefix(const std::string& input, size_t n) {
    std::string result;
    size_t visible_count = 0;    // Count of non-ANSI characters
    bool in_escape = false;      // Track if inside escape sequence
    
    for (size_t i = 0; i < input.size(); ++i) {
        if (!in_escape) {
            // Check for start of ANSI sequence
            if (input[i] == '\033' && i + 1 < input.size() && input[i + 1] == '[') {
                in_escape = true;
                result += input[i];  // Include escape sequence in output
            } else {
                // Regular visible character
                if (visible_count < n) {
                    result += input[i];
                    ++visible_count;
                } else {
                    break;  // Reached desired visible character count
                }
            }
        } else {
            // Inside escape sequence: copy verbatim
            result += input[i];
            if (input[i] == 'm') {
                in_escape = false;  // End of sequence
            }
        }
    }
    
    // Ensure proper termination if string was truncated mid-color
    ensure_ansi_reset(result);
    return result;
}

/**
 * Colorize only the differing portions of two numeric strings
 * 
 * Provides fine-grained colorization by:
 * 1. Splitting numbers into mantissa and exponent (if scientific notation)
 * 2. Finding the first differing position in mantissa
 * 3. Coloring everything from that point onward in red
 * 4. Handling exponent differences appropriately
 * 
 * Examples:
 *   "3.14159" vs "3.14259" -> "3.14\033[31m159\033[0m" vs "3.14\033[31m259\033[0m"
 *   "1.23e5" vs "1.23e6" -> mantissas match but exponents differ
 * 
 * Modifies s1 and s2 in place.
 */
void Formatter::colorize_different_digits(std::string& s1, std::string& s2) {
    // Lambda to split a string into mantissa and exponent at 'e' or 'E'
    auto split_exp = [](const std::string& s) -> std::pair<std::string, std::string> {
        size_t epos = s.find_first_of("eE");
        if (epos == std::string::npos) return {s, ""};  // No exponent
        return {s.substr(0, epos), s.substr(epos)};     // Split at 'e'
    };
    
    auto [mant1, exp1] = split_exp(s1);
    auto [mant2, exp2] = split_exp(s2);

    std::string out1, out2;
    size_t n = std::min(mant1.size(), mant2.size());
    size_t diff_start = n;  // Position where mantissas first differ
    
    // Find the first differing character position in mantissa
    for (size_t i = 0; i < n; ++i) {
        if (mant1[i] != mant2[i]) {
            diff_start = i;
            break;
        }
    }
    
    // If mantissas have different lengths, that's also a difference
    if (mant1.size() != mant2.size()) {
        diff_start = std::min(diff_start, n);
    }
    
    // Build colored mantissas: matching prefix + red differing suffix
    if (diff_start < mant1.size()) {
        out1 = mant1.substr(0, diff_start) + std::string(RED) + mant1.substr(diff_start) +
               std::string(RESET);
    } else {
        out1 = mant1;  // No difference in mantissa1  // No difference in mantissa1
    }
    
    if (diff_start < mant2.size()) {
        out2 = mant2.substr(0, diff_start) + std::string(RED) + mant2.substr(diff_start) +
               std::string(RESET);
    } else {
        out2 = mant2;  // No difference in mantissa2  // No difference in mantissa2
    }
    
    // Handle exponent colorization
    if (!exp1.empty() || !exp2.empty()) {
        // If mantissas differ, color all exponents (even if equal)
        if (diff_start < n || mant1.size() != mant2.size()) {
            if (!exp1.empty()) out1 += std::string(RED) + exp1 + std::string(RESET);
            if (!exp2.empty()) out2 += std::string(RED) + exp2 + std::string(RESET);
        }
        // If mantissas match, only color if exponents differ
        else if (exp1 == exp2) {
            out1 += exp1;  // No color, exponents match
            out2 += exp2;
        } else {
            // Exponents differ
            if (!exp1.empty()) out1 += std::string(RED) + exp1 + std::string(RESET);
            if (!exp2.empty()) out2 += std::string(RED) + exp2 + std::string(RESET);
        }
    }
    
    // Update the input strings with colorized versions
    s1 = out1;
    s2 = out2;
}
