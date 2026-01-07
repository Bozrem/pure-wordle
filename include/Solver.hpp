// TODO: Method documentation

#pragma once

#include "Types.hpp"
#include "Wordle.hpp"
#include "MemoizationTable.hpp"

constexpr int PRUNE_THRESHOLD = 20; // TODO: Add these to a config obj
constexpr double FAIL_COST = 1e9;

struct SearchInfo {
    double expected_cost;
    int max_height;

    bool operator<(const SearchInfo& other) const {
        return expected_cost < other.expected_cost;
    }
};

class Solver {
    const Wordle& game;
    MemoizationTable& cache;

public:
    Solver(const Wordle& g, MemoizationTable& cache);

    SearchInfo evaluate_guess(const StateBitset& state, int guess_ind, const GuessBitset& useful_guesses, int depth);

private:
    // The actual internal recursion
    SearchInfo solve_state(const StateBitset& state, const GuessBitset& useful_guesses, int depth);

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
};
