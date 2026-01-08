// TODO: Method documentation

#pragma once

#include "Wordle.hpp"
#include "Definitions.hpp"
#include "MemoizationTable.hpp"

class Solver {
    const Config& config;
    const Wordle& game;
    MemoizationTable& cache;

public:
    Solver(const Config& c, const Wordle& g, MemoizationTable& m);

    SearchResult evaluate_guess(const StateBitset& state, int guess_ind, const GuessBitset& useful_guesses, int depth);

private:
    // The actual internal recursion
    SearchResult solve_state(const StateBitset& state, const GuessBitset& useful_guesses, int depth);

    GuessBitset prune_actions(const StateBitset& state, const GuessBitset& curr_guesses);

    // Helpers
    template <size_t N>
    std::vector<int> bitset_to_ind_vector(std::bitset<N> set) {
        std::vector<int> active_indices;
        active_indices.reserve(N);
        for(int i = 0; i < N; ++i) {
            if(set.test(i)) active_indices.push_back(i);
        }
        return active_indices;
    }

    static_assert(sizeof(void*) == 8, "Fast bitset iteration requires 64-bit system");

    template<size_t N, typename Func>
    inline void for_each_active_bit(const std::bitset<N>& bitset, Func func) {
        
        // GCC / Linux
        #if defined(__GLIBCXX__) 
            for (size_t i = bitset._Find_first(); i < N; i = bitset._Find_next(i)) {
                func(static_cast<int>(i));
            }

        // Clang / macOS / LLVM
        // Hardware bitscan on the words
        #elif defined(__clang__) || defined(_LIBCPP_VERSION)
            // Cast to raw bits
            const uint64_t* data = reinterpret_cast<const uint64_t*>(&bitset);
            constexpr int num_words = (N + 63) / 64;

            for (int i = 0; i < num_words; ++i) {
                uint64_t word = data[i];

                // Scan for bit in this word
                while (word != 0) {
                    // Find index of first set bit (0-63)
                    int bit_idx = __builtin_ctzll(word); 

                    int global_idx = (i * 64) + bit_idx;
 
                    // Safety check: Ensure we don't read padding bits beyond N
                    if (global_idx < N) {
                        func(global_idx);
                    }

                    // Clear this bit to find the next one
                    word &= (word - 1); 
                }
            }

        // Fallback to loop
        #else
            for (int i = 0; i < (int)N; ++i) {
                if (bitset.test(i)) {
                    func(i);
                }
            }
        #endif
    }
};
