include(FetchContent)

# Download GoogleTest
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
# For Windows: Prevent gtest from overriding shared runtime library settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Create test executable
add_executable(WordleTests WordleTests.cpp)
add_executable(MemoTest MemoizationTableTest.cpp)

# Link WordleCore and GTest
target_link_libraries(WordleTests PRIVATE WordleCore GTest::gtest_main)
target_link_libraries(MemoTest PRIVATE WordleCore GTest::gtest_main)

# Copy patterns csv into build
add_custom_command(TARGET WordleTests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_SOURCE_DIR}/test_patterns.csv
    $<TARGET_FILE_DIR:WordleTests>/test_patterns.csv
    COMMENT "Copying test patterns..."
)

# Register tests with gtest
include(GoogleTest)
gtest_discover_tests(WordleTests)
gtest_discover_tests(MemoTest)
