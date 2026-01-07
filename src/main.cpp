#include "MemoizationTable.hpp"
#include "Solver.hpp"
#include "Types.hpp"
#include "Wordle.hpp"
#include "Statistics.hpp"

#include <chrono>
#include <iostream>

struct RunState {
    int next_guess_index = 0;
    double global_min = 1000; // Just start high
    int best_index = -1; // None yet
};

int main() {
    // TODO: Add config

    Wordle game("data/answers_small.txt", "data/guesses.txt");

    std::cout << "Building Wordle LUT...\n";
    game.build_lut();
    std::cout << "Finished building Wordle LUT\n";

    MemoizationTable cache;
    Solver solver(game, cache); // TODO: Investigate Thread-local advantage

    RunState state;
    // TODO: Checkpoint recovery here

    std::atomic<int> ticket_counter {state.next_guess_index};

    // auto last_checkpoint_ts = std::chrono::steady_clock::now(); // TODO: Checkpointing
    std::mutex save_mutex;

    // These set both to all being possible
    const StateBitset root_state = ~StateBitset();      // Default constructor is all 0. Flip it
    const GuessBitset root_guesses = ~GuessBitset();

    #pragma omp parallel
    {
        while (true) {
            // TODO: Optimization to consider, choose random guesses instead of alphabetical
            // Right now, we probably often get cache misses because words are similar, and a different thread is already working on it
            int guess_ind = ticket_counter.fetch_add(1);
            if (guess_ind >= NUM_GUESSES) break; // This only breaks for THIS thread right?

            SearchInfo res = solver.evaluate_guess(root_state, guess_ind, root_guesses, 1); // This should be 1, and not 0 right?
 
            #pragma omp critical
            {
                std::cout << "Solved " << game.get_guess_str(guess_ind) << " to " << res.expected_cost;
                if (res.expected_cost < state.global_min) {
                    state.global_min = res.expected_cost;
                    state.best_index = guess_ind;
                    std::cout << "\t[NEW BEST]";
                }

                std::cout << '\n';
            }

            // TODO: Checkpoint logic
            // Check the clock, grab mutex, then call MemoizationTable.dump or something
            // MemoizationTable should have a shared mutex that effectively pauses all workers during a checkpoint
        }
    }

    std::cout << "\n\nComputation Complete! Best opener is " << game.get_guess_str(state.best_index)
              << " at " << state.global_min << " expected guesses\n";
    return 0;
}
