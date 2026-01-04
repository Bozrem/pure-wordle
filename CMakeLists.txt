cmake_minimum_required(VERSION 3.15)
project(MCDPWordle VERSION 1.0 LANGUAGES CXX)

# Config
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Dependencies
if(APPLE)
    if(EXISTS "/opt/homebrew/opt/libomp")
        set(OpenMP_ROOT "/opt/homebrew/opt/libomp")
    elseif(EXISTS "/usr/local/opt/libomp")
        set(OpenMP_ROOT "/usr/local/opt/libomp")
    endif()
endif()

find_package(OpenMP REQUIRED)

# Logic is being built as a library to separate it better
add_library(WordleCore
    src/MemoizationTable.cpp
    src/Solver.cpp
    src/Wordle.cpp
    include/BitsetHash.hpp
    include/MemoizationTable.hpp
    include/Solver.hpp
    include/Types.hpp
    include/Wordle.hpp
)

# Public makes it so everything that uses WordleCore also gets the /include
target_include_directories(WordleCore PUBLIC include)
target_link_libraries(WordleCore PRIVATE OpenMP::OpenMP_CXX)

if(MSVC)
    target_compile_options(WordleCore PRIVATE /W4 /O2 /MP)
else()
    target_compile_options(WordleCore PRIVATE -Wall -Wextra -O3 -march=native -funroll-loops)
endif()

# Main executable
add_executable(WordleSolver src/main.cpp)

target_link_libraries(WordleSolver PRIVATE WordleCore)

# Copy data assets to build
add_custom_command(TARGET WordleSolver POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/data
    $<TARGET_FILE_DIR:WordleSolver>/data
    COMMENT "Copying data assets to build directory..."
)

# Testing stuff
enable_testing()
add_subdirectory(tests)
