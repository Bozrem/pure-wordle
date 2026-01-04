#pragma once

#include "Types.hpp"
#include <vector>
#include <string>

using Pattern = uint8_t;

enum class Color : uint8_t {
    Gray = 0,
    Yellow = 1,
    Green = 2
};

class Wordle {
private:
    std::vector<std::string> answers;
    std::vector<std::string> guesses;
    std::vector<uint8_t> pattern_lut;

public:
    Wordle(const std::string& answers_path, const std::string& guesses_path);

    void build_lut();

    static Pattern compute_pattern(const std::string& guess, const std::string& target);

    Pattern get_pattern_lookup(int guess_index, int answer_index) const;

    StateBitset prune_state(const StateBitset& current, int guess_index, int answer_index);

    const std::string& get_guess_str(int index) const { return guesses[index]; }
    const std::string& get_answer_str(int index) const { return answers[index]; }
};
