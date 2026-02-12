#pragma once
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

class Formatter {
public:
    Formatter() = delete;

    static std::vector<size_t> calculate_col_widths(const std::vector<std::string>& t1,
                                        const std::vector<std::string>& t2);

    // Remove ANSI color codes from a string
    static std::string strip_ansi(const std::string& input);

    // Ensure the string ends with the ANSI reset code if the last color set is not reset
    static void ensure_ansi_reset(std::string& str);

    // Extract the first n visible (non-ANSI) characters from a string, preserving formatting codes
    static std::string extract_visible_prefix(const std::string& input, size_t n);

    // Print a string in red (for errors)
    static inline void make_red(std::string& str) noexcept {
        str = std::string(RED) + str + std::string(RESET);
    }

    static inline bool string_is_red(const std::string& str) noexcept {
        return str.find(std::string(RED)) != std::string::npos;
    }

    // Colorize only the digits that differ between s1 and s2 (ANSI red: \033[31m ... \033[0m)
    static void colorize_different_digits(std::string& s1, std::string& s2);

private:
    static constexpr std::string_view RED   = "\033[31m";
    static constexpr std::string_view RESET = "\033[0m";

};