#include "TextParser.hpp"

#include <charconv>
#include <sstream>

std::string TextParser::m_comment_prefix = "#";

void TextParser::set_comment_prefix(const std::string& comment_prefix) {
    m_comment_prefix = comment_prefix;
}

bool TextParser::line_is_comment(const std::string& line) {
    size_t pos = line.find_first_not_of(" \t");
    if (pos == std::string::npos) return false;
    return line.compare(pos, m_comment_prefix.size(), m_comment_prefix) == 0;
}

// Helper: check if a string is numeric
bool TextParser::string_is_numeric(const std::string& str) {
    double value;
    auto result = std::from_chars(str.c_str(), str.c_str() + str.size(), value);

    return result.ec == std::errc() && result.ptr == str.c_str() + str.size();
}

// Helper: tokenize a line into a vector of strings
std::vector<std::string> TextParser::tokenize(const std::string& line) {
    std::istringstream iss(line);
    std::vector<std::string> tokens;
    std::string token;
    while (iss >> token) tokens.push_back(token);
    return tokens;
}