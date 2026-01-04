#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <gtest/gtest.h>

#include "Wordle.hpp"

// Helper to turn "ggy--" into the base 3 pattern Wordle.hpp uses
Pattern parse_pattern_string(const std::string& s) {
    Pattern p = 0;
    int multiplier = 1;
    for (char c : s) {
        int val = 0;
        if (c == 'g') val = 2;
        else if (c == 'y') val = 1;
        p += val * multiplier;
        multiplier *= 3;
    }
    return p;
}

std::string pattern_to_string(Pattern p) {
    std::string s = "";
    for (int i = 0; i < 5; ++i) {
        int val = p % 3;
        if (val == 2) s += 'g';
        else if (val == 1) s += 'y';
        else s += '-';
        p /= 3;
    }
    return s;
}

TEST(WordleLogic, ValidatePatternsWithFile) {
    std::ifstream tests_file("tests/test_patterns.csv");
    ASSERT_TRUE(tests_file.is_open()) << "Couldn't open test_patterns.csv";

    std::string line;
    while (std::getline(tests_file, line)) {
        if (line.empty() || line[0] == '#') continue; // Allows comments to make the tests more readable

        std::stringstream ss(line); // Makes csv parsing easier
        std::string guess, target, pattern_str;

        std::getline(ss, guess, ','); // Dumb syntax. Why would getline allow doing things other than lines?
        std::getline(ss, target, ',');
        std::getline(ss, pattern_str, ',');

        uint8_t expected = parse_pattern_string(pattern_str);
        uint8_t actual = Wordle::compute_pattern(guess, target);

        EXPECT_EQ(actual, expected) 
            << "Failed on Guess: " << guess << ", Target: " << target << "\n"
            << "  Expected: " << pattern_to_string(expected) << " (" << pattern_str << ")\n"
            << "  Actual:   " << pattern_to_string(actual);
    }
}
