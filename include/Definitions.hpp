#pragma once

#include <bitset>
#include <string>

constexpr int NUM_PATTERNS = 243;

// These must match, can only be constexpr
constexpr int NUM_ANSWERS = 50;     // Must match in file
constexpr const char* ANSWERS_PATH = "data/answers_small.txt";

//
constexpr int NUM_GUESSES = 12972;  // Must match in file
constexpr const char* GUESSES_PATH = "data/guesses.txt";

using StateBitset = std::bitset<NUM_ANSWERS>;
using GuessBitset = std::bitset<NUM_GUESSES>;

struct Config {
    std::string answers_path = "data/answers_small.txt";
    std::string guesses_path = "data/guesses.txt";

    int num_threads = 8;
    bool enable_checkpointing = false;

    int agnostic_reserve = 100000;
    int specific_reserve = 100000;

    int prune_threshold = 20;
    double fail_cost = 1e9;
};

extern const Config* g_config;
