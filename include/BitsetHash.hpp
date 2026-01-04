#pragma once

#include "Types.hpp"

struct BitsetHash {
    std::size_t operator()(const StateBitset& state) const noexcept {
        // As a note, I didn't make this, but I think I mostly understand it
        const size_t* p = reinterpret_cast<const size_t*>(&state);

        constexpr size_t bits_per_word = sizeof(size_t) * 8;
        constexpr size_t num_words = (NUM_ANSWERS + bits_per_word - 1) / bits_per_word;

        // Hash combine
        size_t seed = 0xDEADBEEF;
        for (size_t i = 0; i < num_words; ++i) {
            seed ^= p[i] + 0xDEADBEEF + (seed << 6) + (seed >> 2);
        }

        return seed;
    }
};
