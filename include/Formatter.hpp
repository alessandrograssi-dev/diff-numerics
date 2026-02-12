// Formatter.hpp
// -------------------------------------------------------------
// String formatting utilities for diff-numerics
//
// Provides static methods for:
// - ANSI color code manipulation (adding, removing, checking)
// - String width calculation for aligned output
// - Selective digit colorization for highlighting differences
// - Visible character extraction while preserving formatting
// -------------------------------------------------------------

#pragma once
#include <string>
#include <vector>

/**
 * Static utility class for string formatting and ANSI color code handling
 * 
 * Handles all string formatting operations including ANSI escape sequence
 * manipulation, column width calculation, and selective colorization.
 * This class cannot be instantiated (deleted default constructor).
 */
class Formatter {
   public:
    Formatter() = delete;  // No instances allowed

    /**
     * Calculate column widths for aligned output
     * Returns the maximum width for each column across both token vectors
     */
    static std::vector<size_t> calculate_col_widths(const std::vector<std::string>& t1,
                                                    const std::vector<std::string>& t2);

    /**
     * Remove all ANSI escape codes from a string
     * Returns a clean string with only visible characters
     */
    static std::string strip_ansi(const std::string& input);

    /**
     * Ensure string ends with ANSI reset code if it contains unclosed color codes
     * Prevents color bleeding into subsequent output
     */
    static void ensure_ansi_reset(std::string& str);

    /**
     * Extract first n visible characters while preserving ANSI codes
     * Useful for truncating strings to a specific display width without breaking formatting
     */
    static std::string extract_visible_prefix(const std::string& input, size_t n);

    /**
     * Wrap a string in ANSI red color codes
     * Makes the entire string appear in red in terminal output
     */
    static inline void make_red(std::string& str) noexcept {
        str = std::string(RED) + str + std::string(RESET);
    }

    /**
     * Check if a string contains red ANSI color codes
     * Used to determine if differences are present in formatted output
     */
    static inline bool string_is_red(const std::string& str) noexcept {
        return str.find(std::string(RED)) != std::string::npos;
    }

    /**
     * Colorize only the differing digits between two numeric strings
     * Provides fine-grained highlighting by leaving matching prefixes uncolored
     * Handles scientific notation (mantissa and exponent)
     */
    static void colorize_different_digits(std::string& s1, std::string& s2);

   private:
    // ANSI escape codes for terminal coloring
    static constexpr std::string_view RED = "\033[31m";    // Start red text
    static constexpr std::string_view RESET = "\033[0m";   // Reset to default
};