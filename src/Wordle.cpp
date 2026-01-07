#include "Wordle.hpp"
#include "Definitions.hpp"
#include <fstream>
#include <stdexcept>
#include <string>

Wordle::Wordle(const Config& c) : config(c) {
    std::fstream answer_file(ANSWERS_PATH, std::ios::in);
    std::fstream guess_file(GUESSES_PATH, std::ios::in);

    answers.reserve(NUM_ANSWERS);
    guesses.reserve(NUM_GUESSES);

    std::string line;
    while (std::getline(answer_file, line)) { answers.push_back(line); }
    while (std::getline(guess_file, line)) { guesses.push_back(line); }

    if (answers.size() != NUM_ANSWERS)
        throw std::runtime_error("Answers size mismatch: expected " + std::to_string(NUM_ANSWERS) + ", got " + std::to_string(answers.size()));

    if (guesses.size() != NUM_GUESSES)
        throw std::runtime_error("Guesses size mismatch: expected " + std::to_string(NUM_GUESSES) + ", got " + std::to_string(guesses.size()));

    pattern_lut.resize(NUM_GUESSES * NUM_ANSWERS); // Inits all to 0
}

void Wordle::build_lut() {
    #pragma omp parallel for collapse(2)
    for (int g = 0; g < NUM_GUESSES; ++g) {
        for (int a = 0; a < NUM_ANSWERS; ++a) {
            pattern_lut[g * NUM_ANSWERS + a] = compute_pattern(guesses[g], answers[a]); // Shouldn't get out of bounds
        }
    }
} // TODO: Test LUT

Pattern Wordle::compute_pattern(const std::string& guess, const std::string& target) {
    std::array<Color, 5> colors = {Color::Gray, Color::Gray, Color::Gray, Color::Gray, Color::Gray};
    std::array<uint8_t, 26> target_freq = {0};

    for (char c : target) target_freq[c - 'a']++; // the - 'a' lowers a to index 0

    // Green pass
    for (int i = 0; i < 5; ++i) {
        if (guess[i] == target[i]) {
            colors[i] = Color::Green;
            target_freq[target[i] - 'a']--;
        }
    }

    // Yellow pass
    for (int i = 0; i < 5; ++i) {
        if (colors[i] == Color::Green) continue;
        int char_index = guess[i] - 'a';
        if (target_freq[char_index] > 0) {
            colors[i] = Color::Yellow;
            target_freq[char_index]--;
        }
    }

    // Base 3 encode to the pattern
    uint8_t pattern = 0;
    int multiplier = 1;
    for (auto c : colors) {
        pattern += static_cast<uint8_t>(c) * multiplier;
        multiplier *= 3;
    }
    return pattern;
}

const StateBitset Wordle::prune_state(const StateBitset& current, int guess_index, Pattern target_pattern) const {
    StateBitset next_state;
    next_state.reset();

    // TODO: Apply bitmap SIMD optimization
    for (int i = 0; i < NUM_ANSWERS; i++) {
        if (current.test(i)) {
            if (get_pattern_lookup(guess_index, i) == target_pattern)
                next_state.set(i);
        }
    }
    /*
     * This is a pretty core part of the alg that gets run a lot
     * It needs to be highly optimized
     * I think we can do some sort of vectorization to speed it up
     * Need to spend time looking into this
     */

    return next_state;
}



