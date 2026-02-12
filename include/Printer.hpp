#pragma once
#include <ostream>
#include <string>
#include <vector>

namespace numdiff {
struct NumericDiffResult;
struct NumericDiffOptions;
}  // namespace numdiff

class Printer {
   public:
    Printer(std::ostream& os) : os_(os) {}

    void print(const numdiff::NumericDiffResult& result, const numdiff::NumericDiffOptions& opts);

    // New helper: print tokens side by side, column by column, with color and padding
    void print_side_by_side_tokens(const std::vector<std::string>& tokens1,
                                   const std::vector<std::string>& tokens2,
                                   const std::vector<size_t>& col_widths, int line_length);

    // Print differences in a diff-like format
    void print_diff(const std::string& output1, const std::string& output2,
                    const std::string& errors);

   private:
    std::ostream& os_;
};