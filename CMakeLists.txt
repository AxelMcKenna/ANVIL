cmake_minimum_required(VERSION 3.25)

project(anvil
    VERSION 0.1.0
    DESCRIPTION "Limit order book matching engine and market-making backtester"
    LANGUAGES CXX
)

# ---------------------------------------------------------------------------
# Language standard. C++20, strict, no GNU extensions.
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------------------------------------------------------------------------
# Output layout: everything under build/<preset>/bin or /lib.
# ---------------------------------------------------------------------------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ---------------------------------------------------------------------------
# Build options.
# ---------------------------------------------------------------------------
option(ANVIL_BUILD_TESTS      "Build unit tests"          ON)
option(ANVIL_BUILD_BENCHMARKS "Build microbenchmarks"     OFF)

# ---------------------------------------------------------------------------
# Warnings. Aggressive flag set, errors on warning. Shared INTERFACE target
# so every component (library, tests, benchmarks) compiles under the same
# regime without repetition.
# ---------------------------------------------------------------------------
add_library(anvil_warnings INTERFACE)
target_compile_options(anvil_warnings INTERFACE
    -Wall
    -Wextra
    -Wpedantic
    -Werror
    -Wshadow
    -Wconversion
    -Wsign-conversion
    -Wnon-virtual-dtor
    -Wold-style-cast
    -Wcast-align
    -Wunused
    -Woverloaded-virtual
    -Wnull-dereference
    -Wdouble-promotion
    -Wformat=2
)

# ---------------------------------------------------------------------------
# Core library. Sources are added as the project grows. Public headers live
# under include/anvil/ so consumers write #include <anvil/order_book.hpp>.
# ---------------------------------------------------------------------------
add_library(anvil STATIC
    src/version.cpp
    src/price_level.cpp
)
target_include_directories(anvil PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_link_libraries(anvil PRIVATE anvil_warnings)

# ---------------------------------------------------------------------------
# Driver executable. Will grow into the backtester entry point.
# ---------------------------------------------------------------------------
add_executable(anvil_main src/main.cpp)
target_link_libraries(anvil_main PRIVATE anvil anvil_warnings)

# ---------------------------------------------------------------------------
# Tests.
# ---------------------------------------------------------------------------
if(ANVIL_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ---------------------------------------------------------------------------
# Benchmarks (Phase 1 baseline onward; gated until benchmark dep added).
# ---------------------------------------------------------------------------
if(ANVIL_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()
