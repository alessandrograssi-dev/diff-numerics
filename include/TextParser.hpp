#pragma once

#include <string>
#include <vector>

class TextParser {
public:
    TextParser() = delete;

    // Helper: tokenize a line into a vector of strings
    static std::vector<std::string> tokenize(const std::string& line);
    static bool line_is_comment(const std::string& line);
    static bool string_is_numeric(const std::string& str);
    static void set_comment_prefix(const std::string& comment_prefix);

private:
    static std::string m_comment_prefix;
};