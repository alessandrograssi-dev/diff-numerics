// TextParser.hpp
// -------------------------------------------------------------
// Text parsing utilities for diff-numerics
//
// Provides static methods for:
// - Line tokenization (splitting on whitespace)
// - Comment detection (lines starting with specific prefix)
// - Numeric string validation (checking if a string is a valid number)
//
// All operations are stateless and thread-safe.
// -------------------------------------------------------------

#pragma once

#include <string>
#include <string_view>
#include <vector>

/**
 * Static utility class for text parsing operations
 * 
 * Provides fundamental text processing operations used throughout
 * diff-numerics for:
 * - Breaking lines into tokens (whitespace-separated words)
 * - Identifying comment lines to skip during comparison
 * - Validating whether strings represent numeric values
 * 
 * This class cannot be instantiated (deleted default constructor).
 * All methods are static and thread-safe.
 */
class TextParser {
   public:
    TextParser() = delete;  // No instances allowed  // No instances allowed

    /**
     * Split a line into whitespace-separated tokens
     * 
     * Uses standard C++ stream tokenization (operator>>).
     * Consecutive whitespace is treated as a single separator.
     * Leading and trailing whitespace is ignored.
     * 
     * Example: "  1.23   4.56  " -> {"1.23", "4.56"}
     */
    static std::vector<std::string> tokenize(const std::string& line);
    
    /**
     * Check if a line is a comment line
     * 
     * A line is considered a comment if, after skipping leading whitespace,
     * it starts with the specified prefix (e.g., "#", "//", "--").
     * 
     * Empty lines (all whitespace) are not considered comments.
     * 
     * Example:
     *   line = "  # This is a comment", prefix = "#" -> true
     *   line = "123 # inline comment", prefix = "#" -> false
     */
    static bool line_is_comment(const std::string& line, std::string_view prefix);
    
    
    /**  Check if a string represents a valid numeric value */
    static bool string_is_numeric(const std::string& str);
};