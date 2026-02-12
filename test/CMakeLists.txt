# test/CMakeLists.txt
# -------------------------------------------------------------
# CMake configuration for building and running the test suite.
#
# - Fetches and builds GoogleTest using FetchContent
# - Builds the diff-numerics-tests test binary
# - Registers the test with CTest for automated testing
# -------------------------------------------------------------

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_executable(diff-numerics-tests
    ${CMAKE_SOURCE_DIR}/test/test-diff-numerics.cpp
    ${CMAKE_SOURCE_DIR}/src/NumericDiff.cpp
    ${CMAKE_SOURCE_DIR}/src/Formatter.cpp
    ${CMAKE_SOURCE_DIR}/src/Printer.cpp
    ${CMAKE_SOURCE_DIR}/src/TextParser.cpp
)
target_include_directories(diff-numerics-tests PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(diff-numerics-tests gtest_main)
target_compile_definitions(diff-numerics-tests PRIVATE TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
add_test(NAME diff-numerics-tests COMMAND diff-numerics-tests)
