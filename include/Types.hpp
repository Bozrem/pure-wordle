#pragma once
#include <bitset>

constexpr int NUM_ANSWERS = 50;
constexpr int NUM_GUESSES = 12972;

using StateBitset = std::bitset<NUM_ANSWERS>;
using GuessBitset = std::bitset<NUM_GUESSES>;
