#include "MemoizationTable.hpp"
#include "Solver.hpp"
#include "Wordle.hpp"

#include <iostream>

int main() {
    // TODO: Add config

    Wordle game("data/answers_small.txt", "data/guesses.txt");

    std::cout << "Building Wordle LUT...\n";
    game.build_lut(); // TODO: This should be in MemoizationTable for checkpointing
    std::cout << "Finished building the Wordle LUT\n";

    MemoizationTable cache;
    Solver solver(game, cache);

    double global_min = 1000;
    int best_index = -1;

    for (int i = 0; i < NUM_GUESSES; ++i) {
        // Hand off to solver
        double cost = solver.evaluate_opener(i);

        std::cout << "Computed " << game.get_guess_str(i) << " at a cost of " << cost;

        if (cost < global_min) {
            global_min = cost;
            best_index = i;
            std::cout << "\t[NEW BEST!]";
        }

        std::cout << std::endl;

        // TODO: Add checkpointing
        // TODO: Add better statistics
        // TODO: Add progress bar / gui
    }

    std::cout << "\n\nComputation Complete! Best opener is " << game.get_guess_str(best_index) << " at " << global_min << " expected guesses\n";
    return 0;
}
