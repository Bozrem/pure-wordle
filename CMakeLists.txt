# NOTE: Mostly made with AI
cmake_minimum_required(VERSION 3.15)
project(PureWordle VERSION 1.0 LANGUAGES CXX)

# --- CONFIGURATION ---
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Default to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")

# --- DEPENDENCIES ---

# OpenMP (Standard Detection for Linux/Cluster)
find_package(OpenMP REQUIRED)

# Parallel Hashmap
include(FetchContent)
FetchContent_Declare(
    phmap
    URL https://github.com/greg7mdp/parallel-hashmap/archive/refs/tags/v2.0.0.zip
)
FetchContent_MakeAvailable(phmap)

# --- SOURCE MANAGEMENT ---
# Professional Note: Explicitly listing files is preferred over globbing
# because it ensures CMake detects when files are added or removed.
set(CORE_SOURCES
    src/Solver.cpp
    src/Wordle.cpp
    src/MemoizationTable.cpp
)

set(CORE_HEADERS
    include/Definitions.hpp
    include/MemoizationTable.hpp
    include/Solver.hpp
    include/Wordle.hpp
    include/FastBitset.hpp
)

# --- LIBRARY (THE LOGIC) ---
add_library(WordleCore ${CORE_SOURCES} ${CORE_HEADERS})

target_include_directories(WordleCore PUBLIC include)

target_link_libraries(WordleCore PUBLIC OpenMP::OpenMP_CXX phmap)

# --- COMPILE OPTIONS ---
# Base options (Always applied)
target_compile_options(WordleCore PRIVATE
    -Wall -Wextra
    -march=znver2 # Assuming cluster architecture is constant
)

# Configuration-specific options using Generator Expressions
target_compile_options(WordleCore PRIVATE
    # Release Flags: Max Speed
    $<$<CONFIG:Release>:-O3 -funroll-loops>

    # Debug Flags: ASan, Symbols, No Optimization
    $<$<CONFIG:Debug>:-g -O0 -fno-omit-frame-pointer -fsanitize=address>
)

# Linker options (ASan must be linked if compiled with it)
target_link_options(WordleCore PUBLIC
    $<$<CONFIG:Debug>:-fsanitize=address>
)

# --- MAIN EXECUTABLE ---
add_executable(WordleSolver src/main.cpp)

target_link_libraries(WordleSolver PRIVATE WordleCore)

add_custom_command(TARGET WordleSolver POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/data
    $<TARGET_FILE_DIR:WordleSolver>/data
    COMMENT "Copying data assets to build directory..."
)

# --- TESTING ---
enable_testing()
add_subdirectory(tests)
