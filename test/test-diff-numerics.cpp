// test_diff_numerics.cpp
// -------------------------------------------------------------
// This file contains unit tests for the diff-numerics project.
// It uses GoogleTest to verify the behavior of the NumericDiff class,
// which performs numerical comparison between two data files.
//
// Each test case checks a different configuration or option of the
// NumericDiff tool, using sample data files in the test/ directory.
// -------------------------------------------------------------

#include <gtest/gtest.h>

#include <array>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <memory>

#include "NumericDiff.hpp"

namespace fs = std::filesystem;
using namespace numdiff;

// Helper to copy test data to a temp file (not used in current tests)
void copy_file(const std::string& src, const std::string& dst) {
    std::ifstream in(src);
    std::ofstream out(dst);
    out << in.rdbuf();
}

// Helper to get the absolute path to a file relative to the project root
std::string abs_path(const std::string& rel) {
    return (fs::absolute(fs::path("../") / rel)).string();
}

// Helper to get the absolute path to a test data file
#ifndef TEST_DATA_DIR
#error "TEST_DATA_DIR is not defined. Please set it in CMake."
#endif
std::string test_data_path(const std::string& filename) {
    std::string path = std::string(TEST_DATA_DIR) + "/" + filename;
    return path;
}

// Helper to get the project root directory (one level up from test/)
std::string project_root() {
    return fs::absolute(fs::path(TEST_DATA_DIR) / "..").string();
}

// Helper to run NumericDiff and capture its output
// This function constructs a NumericDiff object with the given options,
// runs the comparison, and returns the output as a string.
NumericDiffResult run_diff(const std::string& file1, const std::string& file2, double tol,
                     double threshold, bool side_by_side, bool suppress_common_lines,
                     bool only_equal, bool quiet) {
    testing::internal::CaptureStdout();
    NumericDiffOption opts;
    opts.file1 = file1;
    opts.file2 = file2;
    opts.tolerance = tol;
    opts.threshold = threshold;
    opts.side_by_side = side_by_side;
    opts.suppress_common_lines = suppress_common_lines;
    opts.only_equal = only_equal;
    opts.quiet = quiet;
    NumericDiff diff(opts);
    NumericDiffResult result = diff.run();
    testing::internal::GetCapturedStdout();  // Release the capture
    return result;
}

// Helper to run the diff_numerics binary as a subprocess and capture stderr
std::string run_diff_numerics_cli(const std::string& args) {
    std::array<char, 256> buffer;
    std::string result;
    // Use absolute path to binary and test data
    std::string bin = project_root() + "/build/diff-numerics";
    std::string cmd = bin + " " + args + " 2>&1";
    std::unique_ptr<FILE, int (*)(FILE*)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) return "";
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

// Test: Default tolerance, files should be different (output should not be empty)
TEST(DiffNumerics, DifferentFilesDefaultTolerance) {
    std::string file1 = test_data_path("delta_3D2_2.dat");
    std::string file2 = test_data_path("delta_3D2.dat");
    NumericDiffResult output = run_diff(file1, file2, 1E-2, 1E-6, false, false, false, false);
    EXPECT_NE(output.n_different_lines, 0);
}

// Test: Tight tolerance, files should still be different (output should not be empty)
TEST(DiffNumerics, DifferentFilesTightTolerance) {
    std::string file1 = test_data_path("delta_3D2_2.dat");
    std::string file2 = test_data_path("delta_3D2.dat");
    NumericDiffResult output = run_diff(file1, file2, 1E-10, 1E-12, false, false, false, false);
    EXPECT_NE(output.n_different_lines, 0);

}

// Test: Side-by-side output mode
TEST(DiffNumerics, SideBySideOutput) {
    std::string file1 = test_data_path("delta_3D2_2.dat");
    std::string file2 = test_data_path("delta_3D2.dat");
    NumericDiffResult output = run_diff(file1, file2, 1E-2, 1E-6, true, false, false, false);
    EXPECT_NE(output.n_different_lines, 0);
    // EXPECT_NE(output.find("|"), std::string::npos);
}

// Test: Suppress common lines in output (should still be non-empty for different files)
TEST(DiffNumerics, SuppressCommonLines) {
    std::string file1 = test_data_path("delta_3D2_2.dat");
    std::string file2 = test_data_path("delta_3D2.dat");
    NumericDiffResult output = run_diff(file1, file2, 1E-2, 1E-6, true, true, false, false);
    EXPECT_NE(output.n_different_lines, 0);
}

// Test: Quiet mode (should still be non-empty for different files)
TEST(DiffNumerics, QuietMode) {
    std::string file1 = test_data_path("delta_3D2_2.dat");
    std::string file2 = test_data_path("delta_3D2.dat");
    NumericDiffResult output = run_diff(file1, file2, 1E-2, 1E-6, false, false, false, true);
    EXPECT_NE(output.n_different_lines, 0);
}

// Test: Invalid column width
TEST(DiffNumericsCLI, InvalidColumnWidth) {
    std::string out = run_diff_numerics_cli("-w 5 " + test_data_path("delta_3D2_2.dat") + " " +
                                            test_data_path("delta_3D2.dat"));
    EXPECT_NE(out.find("Error: Column width"), std::string::npos);
    out = run_diff_numerics_cli("-w 500 " + test_data_path("delta_3D2_2.dat") + " " +
                                test_data_path("delta_3D2.dat"));
    EXPECT_NE(out.find("Error: Column width"), std::string::npos);
}

// Test: Invalid tolerance
TEST(DiffNumericsCLI, InvalidTolerance) {
    std::string out = run_diff_numerics_cli("-t 1e-20 " + test_data_path("delta_3D2_2.dat") + " " +
                                            test_data_path("delta_3D2.dat"));
    EXPECT_NE(out.find("Error: Tolerance"), std::string::npos);
    out = run_diff_numerics_cli("-t 1e5 " + test_data_path("delta_3D2_2.dat") + " " +
                                test_data_path("delta_3D2.dat"));
    EXPECT_NE(out.find("Error: Tolerance"), std::string::npos);
}

// Test: Invalid threshold
TEST(DiffNumericsCLI, InvalidThreshold) {
    std::string out = run_diff_numerics_cli("-T -1 " + test_data_path("delta_3D2_2.dat") + " " +
                                            test_data_path("delta_3D2.dat"));
    EXPECT_NE(out.find("Error: Threshold"), std::string::npos);
    out = run_diff_numerics_cli("-T 1e5 " + test_data_path("delta_3D2_2.dat") + " " +
                                test_data_path("delta_3D2.dat"));
    EXPECT_NE(out.find("Error: Threshold"), std::string::npos);
}

// Test: Same file error
TEST(DiffNumericsCLI, SameFileError) {
    std::string out = run_diff_numerics_cli(test_data_path("delta_3D2_2.dat") + " " +
                                            test_data_path("delta_3D2_2.dat"));
    EXPECT_NE(out.find("Error: The two input files must be different."), std::string::npos);
}

// Test: Missing file error
TEST(DiffNumericsCLI, MissingFileError) {
    std::string out = run_diff_numerics_cli(test_data_path("delta_3D2_2.dat"));
    EXPECT_NE(out.find("Error: Two input files must be specified."), std::string::npos);
}

// --- Tests for delta_3P2-3F2.dat and delta_3P2-3F2_2.dat ---

// Test: Default tolerance, files should be different
TEST(DiffNumerics, P2F2_DifferentFilesDefaultTolerance) {
    std::string file1 = test_data_path("delta_3P2-3F2.dat");
    std::string file2 = test_data_path("delta_3P2-3F2_2.dat");
    NumericDiffResult output = run_diff(file1, file2, 1E-2, 1E-6, false, false, false, false);
    EXPECT_NE(output.n_different_lines, 0);
    // EXPECT_NE(output.find("1e+99%"), std::string::npos);
    // EXPECT_NE(output.find("< 0.46500000000000002"), std::string::npos);
    // EXPECT_NE(output.find("> 0.46500000000000002"), std::string::npos);
}

// Test: Tight tolerance, files should still be different
TEST(DiffNumerics, P2F2_DifferentFilesTightTolerance) {
    std::string file1 = test_data_path("delta_3P2-3F2.dat");
    std::string file2 = test_data_path("delta_3P2-3F2_2.dat");
    NumericDiffResult output = run_diff(file1, file2, 1E-10, 1E-12, false, false, false, false);
    EXPECT_NE(output.n_different_lines, 0);
    // EXPECT_NE(output.find("< 0.46500000000000002"), std::string::npos);
}

// Test: Side-by-side output mode
TEST(DiffNumerics, P2F2_SideBySideOutput) {
    std::string file1 = test_data_path("delta_3P2-3F2.dat");
    std::string file2 = test_data_path("delta_3P2-3F2_2.dat");
    NumericDiffResult output = run_diff(file1, file2, 1E-2, 1E-6, true, false, false, false);
    EXPECT_NE(output.n_different_lines, 0);
    // EXPECT_NE(output.find("|"), std::string::npos);
    // EXPECT_NE(output.find("0.46500000000000002"), std::string::npos);
    // EXPECT_NE(output.find("1.20741826972573"), std::string::npos);
}

// Test: Suppress common lines in output
TEST(DiffNumerics, P2F2_SuppressCommonLines) {
    std::string file1 = test_data_path("delta_3P2-3F2.dat");
    std::string file2 = test_data_path("delta_3P2-3F2_2.dat");
    NumericDiffResult output = run_diff(file1, file2, 1E-2, 1E-6, true, true, false, false);
    EXPECT_NE(output.n_different_lines, 0);
    // EXPECT_NE(output.find("|"), std::string::npos);
}

// Test: Quiet mode
TEST(DiffNumerics, P2F2_QuietMode) {
    std::string file1 = test_data_path("delta_3P2-3F2.dat");
    std::string file2 = test_data_path("delta_3P2-3F2_2.dat");
    NumericDiffResult output = run_diff(file1, file2, 1E-2, 1E-6, false, false, false, true);
    EXPECT_NE(output.n_different_lines, 0);
}

// Test: CLI summary output for these files
TEST(DiffNumericsCLI, P2F2_CLISummary) {
    std::string args = std::string("-s ") + test_data_path("delta_3P2-3F2.dat") + " " +
                       test_data_path("delta_3P2-3F2_2.dat");
    std::string out = run_diff_numerics_cli(args);
    EXPECT_NE(out.find("Files DIFFER"), std::string::npos);
    EXPECT_NE(out.find("max percentage error"), std::string::npos);
}

// Test: Colorize only differing digits
TEST(DiffNumerics, ColorDifferentDigits) {
    std::string file1 = test_data_path("delta_3P2-3F2.dat");
    std::string file2 = test_data_path("delta_3P2-3F2_2.dat");
    testing::internal::CaptureStdout();
    NumericDiffOption opts;
    opts.file1 = file1;
    opts.file2 = file2;
    opts.tolerance = 1E-2;
    opts.threshold = 1E-6;
    opts.side_by_side = true;
    opts.color_diff_digits = true;
    NumericDiff diff(opts);
    diff.run();
    std::string output = testing::internal::GetCapturedStdout();
    // Should contain ANSI red color code for only the differing digits
    EXPECT_NE(output.find("\033[31m"), std::string::npos);
    // Should not colorize the entire number if only a few digits differ
    size_t first_red = output.find("\033[31m");
    size_t first_reset = output.find("\033[0m", first_red);
    EXPECT_TRUE(first_red != std::string::npos && first_reset != std::string::npos &&
                first_reset > first_red);
}

// Test: Compare only columns 1, 2, and 4 of delta_3P2-3F2.dat and delta_3P2-3F2_2.dat; expect no
// output (columns are equal)
TEST(DiffNumerics, P2F2_Columns1_2_4_Equal) {
    std::string file1 = test_data_path("delta_3P2-3F2.dat");
    std::string file2 = test_data_path("delta_3P2-3F2_2.dat");
    NumericDiffOption opts;
    opts.file1 = file1;
    opts.file2 = file2;
    opts.tolerance = 1E-2;
    opts.threshold = 1E-6;
    opts.side_by_side = false;  // normal mode
    opts.suppress_common_lines = false;
    opts.only_equal = false;
    opts.quiet = false;
    opts.color_diff_digits = false;
    opts.columns_to_compare = {1, 2, 4};  // columns are 1-based
    testing::internal::CaptureStdout();
    NumericDiff diff(opts);
    diff.run();
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_TRUE(output.empty());
}

// Add more tests for different tolerances, thresholds, and options as needed
