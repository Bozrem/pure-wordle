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
};
