# CMakeLists.txt
# -------------------------------------------------------------
# Top-level CMake configuration for the diff-numerics project.
#
# - Sets up project metadata and C++ standard
# - Configures build options and output directories
# - Adds the main executable and install rules
# - Integrates GoogleTest for automated testing
# - Installs man page and headers
# -------------------------------------------------------------

cmake_minimum_required(VERSION 3.10)

project(diff-numerics
    VERSION 1.0
    DESCRIPTION "A C++ project for numerical diff between two files"
    LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Debug vs Release configuration
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -Werror")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

# Include directories for headers
include_directories(include)

# Gather source and header files
set(SOURCES 
    src/main.cpp
    src/ArgParser.cpp
    src/NumericDiff.cpp
)

# Set project version
set(PROJECT_VERSION "1.0.0")

# Add version definition for the compiler
add_definitions(-DNUMERIC_DIFF_VERSION=\"${PROJECT_VERSION}\")

# Main executable target
add_executable(diff-numerics ${SOURCES})
target_compile_options(diff-numerics PRIVATE
    -Wall -Wextra -Wpedantic -Wshadow
    -Wconversion -Wsign-conversion -Wfloat-equal
    -fPIC
)

# Set output directory for binaries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Install rules for binary, man page, and headers
install(TARGETS diff-numerics DESTINATION bin)
install(FILES ${CMAKE_SOURCE_DIR}/diff-numerics.1 DESTINATION share/man/man1)

# Enable testing and add GoogleTest for tests
include(CTest)
enable_testing()
add_subdirectory(test)

# Detect if using Clang and set standard library if needed
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(STATUS "Detected Clang: ${CMAKE_CXX_COMPILER}")
    # Use libstdc++ (default on Linux). To use libc++, uncomment the next line:
    # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    # Optionally, specify include/library paths if needed:
    # include_directories(/usr/include/c++/12)
    # link_directories(/usr/lib/gcc/x86_64-linux-gnu/12)
endif()