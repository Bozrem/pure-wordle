#pragma once

#include "Wordle.hpp"
#include "MemoizationTable.hpp"


class Solver {
    const Wordle& game;
    MemoizationTable& cache;

public:
    Solver(const Wordle& g, MemoizationTable& cache);

    // This is one thread looking at the cost of one opening
    double evaluate_opener(int guess_index);

private:
    // The actual internal recursion
    double solve_state(const StateBitset& state);
};
