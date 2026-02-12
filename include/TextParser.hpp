#pragma once

#include <string>
#include <string_view>
#include <vector>

class TextParser {
   public:
    TextParser() = delete;

    // Helper: tokenize a line into a vector of strings
    static std::vector<std::string> tokenize(const std::string& line);
    static bool line_is_comment(const std::string& line, std::string_view prefix);
    static bool string_is_numeric(const std::string& str);
};