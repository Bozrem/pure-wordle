cmake_minimum_required(VERSION 3.15)
project(MCDPWordle VERSION 1.0 LANGUAGES CXX)

# --- CONFIGURATION ---

# Use C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Default to Release build (High optimization)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# --- DEPENDENCIES ---

# Fix for macOS/Homebrew OpenMP detection
if(APPLE)
    if(EXISTS "/opt/homebrew/opt/libomp")
        set(OpenMP_ROOT "/opt/homebrew/opt/libomp")
    elseif(EXISTS "/usr/local/opt/libomp")
        set(OpenMP_ROOT "/usr/local/opt/libomp")
    endif()
endif()

find_package(OpenMP REQUIRED)

# --- TARGET ---

# Explicitly listing sources is safer than GLOB (prevents stale builds)
add_executable(WordleSolver
    src/main.cpp
    src/MemoizationTable.cpp
    src/Solver.cpp
    src/Wordle.cpp
)

# --- INCLUDES & LINKING ---

target_include_directories(WordleSolver PRIVATE include)

target_link_libraries(WordleSolver PRIVATE OpenMP::OpenMP_CXX)

# --- COMPILER FLAGS ---

# Target-specific compile options (does not pollute global flags)
if(MSVC)
    target_compile_options(WordleSolver PRIVATE /W4 /O2 /MP)
else()
    target_compile_options(WordleSolver PRIVATE -Wall -Wextra -O3 -march=native -funroll-loops)
endif()

# --- ASSETS ---

# Automatically copy the 'data' folder to the build directory after compiling.
# This ensures ./WordleSolver can find "data/answers.txt" without manual copying.
add_custom_command(TARGET WordleSolver POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/data
    $<TARGET_FILE_DIR:WordleSolver>/data
    COMMENT "Copying data assets to build directory..."
)

# --- INFO ---
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
