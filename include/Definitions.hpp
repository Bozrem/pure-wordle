#pragma once

#include <string>

#include "FastBitset.hpp"

constexpr int NUM_PATTERNS = 243;

// These must match, can only be constexpr
constexpr int NUM_ANSWERS = 50;     // Must match in file
constexpr const char* ANSWERS_PATH = "data/answers_small.txt";

//
constexpr int NUM_GUESSES = 12972;  // Must match in file
constexpr const char* GUESSES_PATH = "data/guesses.txt";

using StateBitset = FastBitset<NUM_ANSWERS>;
using GuessBitset = FastBitset<NUM_GUESSES>;

struct SearchResult {
    double expected_cost;
    int best_guess_index; // -1 if not applicable (e.g. leaf node)
    int max_height;       // Depth of the subtree relative to this node

    // Helper for comparisons if you switch to std::min_element later
    bool operator<(const SearchResult& other) const {
        return expected_cost < other.expected_cost;
    }
};

struct Config {
    std::string answers_path = "data/answers_small.txt";
    std::string guesses_path = "data/guesses.txt";

    int num_threads = 8;
    bool enable_checkpointing = false;

    int agnostic_reserve = 100000;
    int specific_reserve = 100000;

    int prune_threshold = 20;
    double fail_cost = 1e9;

    int stats_print_freq = 2000;
};
