#include "MemoizationTable.hpp"
#include "Solver.hpp"
#include "Wordle.hpp"
#include "Statistics.hpp"
#include "Definitions.hpp"

// #include <chrono>
#include <iostream>
#include <numeric>
#include <algorithm>
#include <random>

SolverStats g_stats;

struct RunState {
    int next_guess_index = 0;
    double global_min = 1000; // Just start high
    int best_index = -1; // None yet
};

const Config parse_args(int argc, char** argv) {
    return {};
}

int main(int argc, char** argv) {
    const Config config = parse_args(argc, argv);
    std::cout << "Parsed Config\n";

    Wordle game(config);

    game.build_lut();
    std::cout << "Build LUT\n";

    std::vector<int> task_order(NUM_GUESSES);
    std::iota(task_order.begin(), task_order.end(), 0);
    std::mt19937 rng(67); // hehe
    std::shuffle(task_order.begin(), task_order.end(), rng);
    std::cout << "Shuffled Task Order\n";

    MemoizationTable cache(config);
    Solver solver(config, game, cache);
    g_stats = SolverStats();

    RunState state;
    // TODO: Checkpoint recovery here

    std::atomic<int> ticket_counter {state.next_guess_index};

    // auto last_checkpoint_ts = std::chrono::steady_clock::now(); // TODO: Checkpointing
    std::mutex save_mutex;

    // These set both to all being possible
    StateBitset root_state = StateBitset();
    root_state.set();
    GuessBitset root_guesses = GuessBitset();
    root_guesses.set();

    std::cout << "Starting Simulation\n\n";

    #pragma omp parallel
    {
        t_stats = SolverStats(); // Every thread gets it's own

        while (true) {
            int ticket = ticket_counter.fetch_add(1);
            if (ticket >= NUM_GUESSES) break;

            int guess_ind = task_order[ticket];

            SearchResult res = solver.evaluate_guess(root_state, guess_ind, root_guesses, 1);

            #pragma omp critical
            {
                g_stats += t_stats;
                std::cout << "Solved " << game.get_guess_str(guess_ind) << " to " << res.expected_cost;
                if (res.expected_cost < state.global_min) {
                    state.global_min = res.expected_cost;
                    state.best_index = guess_ind;
                    std::cout << "\t[NEW BEST]";
                }
                std::cout << '\n';

                // if (ticket_counter % config.stats_print_freq == 0)
                //     g_stats.print();
            }

            // TODO: Checkpoint logic
            // Check the clock, grab mutex, then call MemoizationTable.dump or something
            // MemoizationTable should have a shared mutex that effectively pauses all workers during a checkpoint
        }
    }

    std::cout << "\n\nComputation Complete! Best opener is " << game.get_guess_str(state.best_index)
              << " at " << state.global_min << " expected guesses\n";

    g_stats.print();

    return 0;
}
