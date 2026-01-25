# NOTE: Mostly made with AI
cmake_minimum_required(VERSION 3.15)
project(MCDPWordle VERSION 1.0 LANGUAGES CXX)

# --- CONFIGURATION ---
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# --- DEPENDENCIES ---

# OpenMP
if(APPLE)
    if(EXISTS "/opt/homebrew/opt/libomp")
        set(OpenMP_ROOT "/opt/homebrew/opt/libomp")
    elseif(EXISTS "/usr/local/opt/libomp")
        set(OpenMP_ROOT "/usr/local/opt/libomp")
    endif()
endif()
find_package(OpenMP REQUIRED)

# Parallel Hashmap
include(FetchContent)
FetchContent_Declare(
    phmap
    URL https://github.com/greg7mdp/parallel-hashmap/archive/refs/tags/v2.0.0.zip
)
FetchContent_MakeAvailable(phmap)

# --- LIBRARY (THE LOGIC) ---
add_library(WordleCore
    src/Solver.cpp
    src/Wordle.cpp
    src/MemoizationTable.cpp
    include/Definitions.hpp
    include/MemoizationTable.hpp
    include/Solver.hpp
    include/Wordle.hpp
    include/FastBitset.hpp
)

target_include_directories(WordleCore PUBLIC include)

target_link_libraries(WordleCore PUBLIC OpenMP::OpenMP_CXX phmap)

target_compile_options(WordleCore PUBLIC 
    -Wall -Wextra 
    -O3 -march=znver2 -funroll-loops
    -g -fno-omit-frame-pointer
    -fsanitize=address
)
target_link_options(WordleCore PUBLIC -fsanitize=address)

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
