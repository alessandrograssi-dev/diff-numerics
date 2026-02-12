// TextParser.cpp
// -------------------------------------------------------------
// Implementation of text parsing utilities for diff-numerics
//
// Provides core text processing: tokenization, comment detection,
// and numeric validation using modern C++ features (from_chars).
// -------------------------------------------------------------

#include "TextParser.hpp"

#include <charconv>
#include <sstream>

/**
 * Determine if a line is a comment line
 * 
 * Algorithm:
 * 1. Find first non-whitespace character
 * 2. Check if the line starts with the comment prefix at that position
 * 
 * This allows for leading whitespace before comments.
 * Empty or all-whitespace lines return false.
 */
bool TextParser::line_is_comment(const std::string& line, std::string_view prefix) {
    // Skip leading whitespace
    size_t pos = line.find_first_not_of(" \t");
    
    // Empty line or all whitespace: not a comment
    if (pos == std::string::npos) return false;
    
    // Check if prefix matches at first non-whitespace position
    return line.compare(pos, std::string(prefix).size(), std::string(prefix)) == 0;
}

/**
 * Validate whether a string represents a numeric value
 * 
 * Uses C++17's std::from_chars for:
 * - High performance (no locale, no exceptions, no memory allocation)
 * - Exact parsing (returns where parsing stopped)
 * - Support for all numeric formats (int, float, scientific notation)
 * 
 * The string must be entirely consumed for successful validation.
 * Partial numeric strings like "123abc" are rejected.
 */
bool TextParser::string_is_numeric(const std::string& str) {
    double value;  // Parse target (actual value not used, only validity checked)
    
    // Attempt to parse the entire string as a double
    auto result = std::from_chars(str.c_str(), str.c_str() + str.size(), value);

    // Success requires: no error AND parsing consumed entire string
    return result.ec == std::errc() && result.ptr == str.c_str() + str.size();
}

/**
 * Tokenize a line into whitespace-separated tokens
 * 
 * Uses std::istringstream with operator>> for standard whitespace tokenization.
 * This approach:
 * - Treats any sequence of whitespace (space, tab, newline) as separator
 * - Automatically trims leading and trailing whitespace
 * - Splits on consecutive whitespace without creating empty tokens
 * 
 * Returns a vector of tokens in the order they appear.
 * Empty or all-whitespace lines return an empty vector.
 */
std::vector<std::string> TextParser::tokenize(const std::string& line) {
    std::istringstream iss(line);
    std::vector<std::string> tokens;
    std::string token;
    
    // operator>> automatically handles whitespace separation
    while (iss >> token) tokens.push_back(token);
    
    return tokens;
}